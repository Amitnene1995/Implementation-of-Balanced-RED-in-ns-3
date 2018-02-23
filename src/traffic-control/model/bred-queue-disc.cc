/*Copyright

NITK Surathkal

Authors : Amit V. Nene

Mentors : Dr. Mohit P. Tahiliani

*/

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "red-queue-disc.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/net-device-queue-interface.h"
#include <map>
#include <iostream>
#include <string.h>
#include <math.h>
#include <ns3/ipv4-packet-filter.h>
#include <ns3/ipv4-queue-disc-item.h>
#include <ns3/ipv6-packet-filter.h>
#include <ns3/ipv6-queue-disc-item.h>
#include "bred-queue-disc.h"
namespace ns3 {
  std::map<Ipv4Address,unsigned int> qlen_v4;
  std::map<Ipv6Address,unsigned int> qlen_v6;
  uint32_t version;
  double itemSize;
  Ipv4Header hdr;
  Ipv4Address src;
  Ipv6Header hdr1;
  Ipv6Address src1;
  Ptr<Ipv4QueueDiscItem> ipv4Item;
  Ptr<Ipv6QueueDiscItem> ipv6Item;
  Ptr<QueueDiscItem> item;

NS_LOG_COMPONENT_DEFINE ("BredQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (BredQueueDisc);

TypeId BredQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BredQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<BredQueueDisc> ()
    .AddAttribute ("Mode",
                   "Determines unit for QueueLimit",
                   EnumValue (QUEUE_DISC_MODE_PACKETS),
                   MakeEnumAccessor (&BredQueueDisc::SetMode),
                   MakeEnumChecker (QUEUE_DISC_MODE_BYTES, "QUEUE_DISC_MODE_BYTES",
                                    QUEUE_DISC_MODE_PACKETS, "QUEUE_DISC_MODE_PACKETS"))
    .AddAttribute ("MeanPktSize",
                   "Average of packet size",
                   UintegerValue (500),
                   MakeUintegerAccessor (&BredQueueDisc::m_meanPktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("IdlePktSize",
                   "Average packet size used during idle times.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BredQueueDisc::m_idlePktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("UseEcn",
                   "True to use ECN (packets are marked instead of being dropped)",
                   BooleanValue (true),
                   MakeBooleanAccessor (&BredQueueDisc::m_useEcn),
                   MakeBooleanChecker ())
    .AddAttribute ("UseHardDrop",
                   "True to always drop packets above max threshold",
                   BooleanValue (true),
                   MakeBooleanAccessor (&BredQueueDisc::m_useHardDrop),
                   MakeBooleanChecker ())
    .AddAttribute ("QueueLimit",
                   "Buffer Size",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&BredQueueDisc::SetQueueLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("AlphaBRED",
                   "Alpha",
                   DoubleValue (1.5),
                   MakeDoubleAccessor (&BredQueueDisc::SetAlpha),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("BetaBRED",
                   "Beta",
                   DoubleValue (0.75),
                   MakeDoubleAccessor (&BredQueueDisc::SetBeta),
                   MakeDoubleChecker <double> ())
    .AddAttribute ("LinkBandwidth", 
                   "The RED link bandwidth",
                   DataRateValue (DataRate ("1.5Mbps")),
                   MakeDataRateAccessor (&BredQueueDisc::m_linkBandwidth),
                   MakeDataRateChecker ())
  ;

  return tid;
}

BredQueueDisc::BredQueueDisc ()
  : QueueDisc ()
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<UniformRandomVariable> ();
}

BredQueueDisc::~BredQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
BredQueueDisc::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_uv=0;
  QueueDisc::DoDispose ();
}

void
BredQueueDisc::SetMode (QueueDiscMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

BredQueueDisc::QueueDiscMode
BredQueueDisc::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

void
BredQueueDisc::SetQueueLimit (uint32_t lim)
{
  NS_LOG_FUNCTION (this << lim);
  m_queueLimit = lim;
}

void
BredQueueDisc::SetAlpha (double a)
{
  NS_LOG_FUNCTION (this << a);
  m_AlphaBred = a;
}

void
BredQueueDisc::SetBeta (double b)
{
  NS_LOG_FUNCTION (this << b);
  m_BetaBred = b;
}

int64_t 
BredQueueDisc::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uv->SetStream (stream);
  return 1;
}

bool
BredQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  uint32_t nQueued = 0;

  if (GetMode () == QUEUE_DISC_MODE_BYTES)
    {
      NS_LOG_DEBUG ("Enqueue in bytes mode");
      nQueued = GetInternalQueue (0)->GetNBytes ();
    }

  else if (GetMode () == QUEUE_DISC_MODE_PACKETS)
    {
      NS_LOG_DEBUG ("Enqueue in packets mode");
      nQueued = GetInternalQueue (0)->GetNPackets ();
    }

  //uint32_t m = 0;

  if (m_idle == 1)
    {
      NS_LOG_DEBUG ("BRED Queue Disc is idle.");
      Time now = Simulator::Now ();


      //m = uint32_t (m_ptc * (now - m_idleTime).GetSeconds ());


      m_idle = 0;
    }

  double u = m_uv->GetValue ();
  
  m_Nestimate = (1 - m_wn) * m_Nestimate + m_wn * m_Nactive;

  m_p2 = std::sqrt (m_Nestimate) / (std::sqrt (m_Nestimate) + 10);

  m_l2 = m_queueLimit / (2 * std::ceil (m_Nestimate));

  m_p1 = m_p2 / 10;

  m_l1 = m_BetaBred * m_l2;

  m_Wm = m_AlphaBred * m_l2;

  if (DynamicCast<Ipv4QueueDiscItem> (item) != 0)
    {
      version = 4;
    }

  else if (DynamicCast<Ipv6QueueDiscItem> (item) != 0)
    {
      version = 6;
    }

  if (version == 4)
    {
      ipv4Item = DynamicCast<Ipv4QueueDiscItem> (item);

      NS_ASSERT (ipv4Item != 0);

      hdr = ipv4Item->GetHeader ();
      src = hdr.GetSource ();

    }

  else if (version == 6)
    {
      ipv6Item = DynamicCast<Ipv6QueueDiscItem> (item);

      NS_ASSERT (ipv6Item != 0);

      hdr1 = ipv6Item->GetHeader ();
      src1 = hdr1.GetSourceAddress ();

    }

  itemSize = item->GetSize ();
  if ((nQueued + itemSize) <= m_queueLimit)
    {
      if (version == 4)
        {
          if (qlen_v4[src] >= m_l1 && qlen_v4[src] < m_l2)
            {

              if (m_p1 > u && (!m_useEcn || !Mark (item, UNFORCED_MARK)))
                {
                  NS_LOG_DEBUG ("\t Dropping due to Prob Mark ");
                  DropBeforeEnqueue (item, UNFORCED_DROP);
                  return false;
                }
            }

          else if (qlen_v4[src] >= m_l2 && qlen_v4[src] < m_Wm)
            {

              if (m_p2 > u && (!m_useEcn || !Mark (item, UNFORCED_MARK)))
                {
                  NS_LOG_DEBUG ("\t Dropping due to Prob Mark ");
                  DropBeforeEnqueue (item, UNFORCED_DROP);
                  return false;
                }
            }

          else if (qlen_v4[src] > m_Wm && (m_useHardDrop || !m_useEcn || !Mark (item, FORCED_MARK)))
            {
              NS_LOG_DEBUG ("\t Dropping due to Hard Mark ");
              DropBeforeEnqueue (item, FORCED_DROP);
              return false;
            }
        }

      else if (version == 6)
        {
          if (qlen_v6[src1] >= m_l1 && qlen_v6[src1] < m_l2)
            {

              if (m_p1 > u && (!m_useEcn || !Mark (item, UNFORCED_MARK)))
                {
                  NS_LOG_DEBUG ("\t Dropping due to Prob Mark ");
                  DropBeforeEnqueue (item, UNFORCED_DROP);
                  return false;
                }
            }

          else if (qlen_v6[src1] >= m_l2 && qlen_v6[src1] < m_Wm)
            {

              if (m_p2 > u && (!m_useEcn || !Mark (item, UNFORCED_MARK)))
                {
                  NS_LOG_DEBUG ("\t Dropping due to Prob Mark ");
                  DropBeforeEnqueue (item, UNFORCED_DROP);
                  return false;
                }
            }

          else if (qlen_v6[src1] > m_Wm && (m_useHardDrop || !m_useEcn || !Mark (item, FORCED_MARK)))
            {
              NS_LOG_DEBUG ("\t Dropping due to Hard Mark ");
              DropBeforeEnqueue (item, FORCED_DROP);
              return false;
            }
        }
    }

  else
    {
      NS_LOG_DEBUG ("\t Dropping due to Hard Mark ");
      DropBeforeEnqueue (item, FORCED_DROP);
      return false;
    }


  bool retval = GetInternalQueue (0)->Enqueue (item);

  // If Queue::Enqueue fails, QueueDisc::DropBeforeEnqueue is called by the
  // internal queue because QueueDisc::AddInternalQueue sets the trace callback

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

  if (retval && version == 4)
    {
      qlen_v4[src] += itemSize;
    }

  else if (retval && version == 4)
    {
      qlen_v6[src1] += itemSize;
    }

  if (version == 4)
    {
      if (qlen_v4[src] == itemSize)
        {
          m_Nactive += 1;
        }
    }

  else if (version == 6)
    {
      if (qlen_v6[src1] == itemSize)
        {
          m_Nactive += 1;
        }
    }
  return retval;

}

void
BredQueueDisc::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Initializing BRED params.");

  m_ptc = m_linkBandwidth.GetBitRate () / (8.0 * m_meanPktSize);

  m_Nactive=0;
  m_N=0;
  m_Nestimate=0;
  m_wn=0.02;
  m_idle=1;
}

uint32_t
BredQueueDisc::GetQueueSize (void)
{
  NS_LOG_FUNCTION (this);
  if (GetMode () == QUEUE_DISC_MODE_BYTES)
    {
      return GetInternalQueue (0)->GetNBytes ();
    }
  else if (GetMode () == QUEUE_DISC_MODE_PACKETS)
    {
      return GetInternalQueue (0)->GetNPackets ();
    }
  else
    {
      NS_ABORT_MSG ("Unknown BRED mode.");
    }
}

Ptr<QueueDiscItem>
BredQueueDisc::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  if (GetInternalQueue (0)->IsEmpty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      m_idle = 1;
      m_idleTime = Simulator::Now ();

      return 0;
    }
  else
    {
      m_idle = 0;
      item = GetInternalQueue (0)->Dequeue ();

      if (version == 4)
        {
          ipv4Item = DynamicCast<Ipv4QueueDiscItem> (item);

          NS_ASSERT (ipv4Item != 0);

          hdr = ipv4Item->GetHeader ();
          src = hdr.GetSource ();

          itemSize = item->GetSize ();

          qlen_v4[src] -= itemSize;

          if (qlen_v4[src] == 0)
            {
              m_Nactive -= 1;
            }

        }

      else if (version == 6)
        {
          ipv6Item = DynamicCast<Ipv6QueueDiscItem> (item);

          NS_ASSERT (ipv6Item != 0);

          hdr1 = ipv6Item->GetHeader ();
          src1 = hdr1.GetSourceAddress ();

          itemSize = item->GetSize ();

          qlen_v6[src1] -= itemSize;

          if (qlen_v6[src1] == 0)
            {
              m_Nactive -= 1;
            }
        }
      NS_LOG_LOGIC ("Popped " << item);

      NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

      return item;
    }
}

Ptr<const QueueDiscItem>
BredQueueDisc::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);
  if (GetInternalQueue (0)->IsEmpty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<const QueueDiscItem> item = GetInternalQueue (0)->Peek ();

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

  return item;
}

bool
BredQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("BredQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("BredQueueDisc cannot have packet filters");
      return false;
    }

  if (GetNInternalQueues () == 0)
    {
      // create a DropTail queue
      Ptr<InternalQueue> queue = CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> > ("Mode", EnumValue (m_mode));
      if (m_mode == QUEUE_DISC_MODE_PACKETS)
        {
          queue->SetMaxPackets (m_queueLimit);
        }
      else
        {
          queue->SetMaxBytes (m_queueLimit);
        }
      AddInternalQueue (queue);


    }

  if (GetNInternalQueues () != 1)
    {
      NS_LOG_ERROR ("BredQueueDisc needs 1 internal queue");
      return false;
    }

  if ((GetInternalQueue (0)->GetMode () == QueueBase::QUEUE_MODE_PACKETS && m_mode == QUEUE_DISC_MODE_BYTES)
      || (GetInternalQueue (0)->GetMode () == QueueBase::QUEUE_MODE_BYTES && m_mode == QUEUE_DISC_MODE_PACKETS))
    {
      NS_LOG_ERROR ("The mode of the provided queue does not match the mode set on the BredQueueDisc");
      return false;
    }

  if ((m_mode ==  QUEUE_DISC_MODE_PACKETS && GetInternalQueue (0)->GetMaxPackets () != m_queueLimit)
      || (m_mode ==  QUEUE_DISC_MODE_BYTES && GetInternalQueue (0)->GetMaxBytes () != m_queueLimit))
    {
      NS_LOG_ERROR ("The size of the internal queue differs from the queue disc limit");
      return false;
    }



  return true;
}

} // namespace ns3
