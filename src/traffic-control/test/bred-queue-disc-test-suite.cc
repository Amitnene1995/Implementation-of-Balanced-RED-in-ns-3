/*Copyright

NITK Surathkal

Authors : Amit V. Nene

Mentors : Dr. Mohit P. Tahiliani

*/

#include "ns3/queue.h"
#include "ns3/test.h"
#include "ns3/bred-queue-disc.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-packet-filter.h"
#include "ns3/ipv4-queue-disc-item.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-packet-filter.h"
#include "ns3/ipv6-queue-disc-item.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/pointer.h"
#include "ns3/packet-burst.h"
#include "ns3/queue-disc.h"
#include "ns3/object-factory.h"

using namespace ns3;

class BredQueueDiscTestItem : public QueueDiscItem
{
public:
  
  BredQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol, bool ecnCapable);
  virtual ~BredQueueDiscTestItem ();
  virtual void AddHeader (void);
  virtual bool Mark(void);

  private:
  BredQueueDiscTestItem ();
  
  BredQueueDiscTestItem (const BredQueueDiscTestItem &);
 
  BredQueueDiscTestItem &operator = (const BredQueueDiscTestItem &);
  bool m_ecnCapablePacket; ///< ECN capable packet?

};

BredQueueDiscTestItem::BredQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol, bool ecnCapable)
  : QueueDiscItem (p, addr, protocol),
    m_ecnCapablePacket (ecnCapable)
{
}

BredQueueDiscTestItem::~BredQueueDiscTestItem ()
{
}

void
BredQueueDiscTestItem::AddHeader (void)
{
}

bool
BredQueueDiscTestItem::Mark (void)
{
  if (m_ecnCapablePacket)
    {
      return true;
    }
  return false;
}

class BredQueueDiscTestCase : public TestCase
{
public:
  BredQueueDiscTestCase ();
  virtual void DoRun (void);
private:
  
  void Enqueue (Ptr<BredQueueDisc> queue, uint32_t size, uint32_t nPkt, bool ecnCapable);
  
  void RunBredTest (StringValue mode);

  void AddPacket (Ptr<BredQueueDisc> queue, Ipv4Header dscp);
};

BredQueueDiscTestCase::BredQueueDiscTestCase ()
  : TestCase ("Sanity check on the balanced red queue implementation")
{
}

void
BredQueueDiscTestCase::AddPacket (Ptr<BredQueueDisc> queue, Ipv4Header dscp)
{
Ptr<Packet> p = Create<Packet> (100);
Ipv4Header ipHeader;
ipHeader.SetPayloadSize (0);
ipHeader.SetProtocol (6);
Address dest;
Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem> (p, dest, 0, ipHeader);
queue->Enqueue (item);
}


void
BredQueueDiscTestCase::RunBredTest (StringValue mode)
{
Ptr<BredQueueDisc> queue = CreateObjectWithAttributes<BredQueueDisc> ();
Ptr<Packet> p1,p2,p3,p4,p5,p6;
NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
uint32_t pktSize = 200;
//test 1
p1 = Create<Packet> (pktSize);
p2 = Create<Packet> (pktSize);
p3 = Create<Packet> (pktSize);
p4 = Create<Packet> (pktSize);
p5 = Create<Packet> (pktSize);
p6 = Create<Packet> (pktSize); 
Address dest;
queue->Initialize();

queue->Enqueue(Create<BredQueueDiscTestItem>(p1,dest,0,false));
queue->Enqueue(Create<BredQueueDiscTestItem>(p2,dest,0,false));
queue->Enqueue(Create<BredQueueDiscTestItem>(p3,dest,0,false));
queue->Enqueue(Create<BredQueueDiscTestItem>(p4,dest,0,false));
queue->Enqueue(Create<BredQueueDiscTestItem>(p5,dest,0,false));
queue->Enqueue(Create<BredQueueDiscTestItem>(p6,dest,0,false));

Ptr<QueueDiscItem> item;

item = queue->Dequeue();
NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the first packet");

item = queue->Dequeue();
item = queue->Dequeue();
item = queue->Dequeue();
item = queue->Dequeue();
item = queue->Dequeue();
item = queue->Dequeue();
NS_TEST_EXPECT_MSG_EQ ((item == 0), true, "I have removed final packet");

//test 2

Ipv4Header hdr,hdr2,hdr3,hdr4,hdr5,hdr6,hdr7,hdr8,hdr9,hdr10;
  hdr.SetPayloadSize (0);
  hdr.SetSource (Ipv4Address ("10.10.1.1"));
  hdr.SetDestination (Ipv4Address ("10.10.1.2"));
  hdr.SetProtocol (7);

  hdr2.SetPayloadSize (0);
  hdr2.SetSource (Ipv4Address ("10.10.1.3"));
  hdr2.SetDestination (Ipv4Address ("10.10.1.7"));
  hdr2.SetProtocol (7); 

  hdr3.SetPayloadSize (0);
  hdr3.SetSource (Ipv4Address ("10.10.1.4"));
  hdr3.SetDestination (Ipv4Address ("10.10.1.8"));
  hdr3.SetProtocol (7);

  hdr4.SetPayloadSize (0);
  hdr4.SetSource (Ipv4Address ("10.10.1.5"));
  hdr4.SetDestination (Ipv4Address ("10.10.1.9"));
  hdr4.SetProtocol (7); 

  hdr5.SetPayloadSize (0);
  hdr5.SetSource (Ipv4Address ("10.10.1.10"));
  hdr5.SetDestination (Ipv4Address ("10.10.1.11"));
  hdr5.SetProtocol (7); 

  hdr6.SetPayloadSize (0);
  hdr6.SetSource (Ipv4Address ("10.10.1.12"));
  hdr6.SetDestination (Ipv4Address ("10.10.1.13"));
  hdr6.SetProtocol (7); 

  hdr7.SetPayloadSize (0);
  hdr7.SetSource (Ipv4Address ("10.10.1.14"));
  hdr7.SetDestination (Ipv4Address ("10.10.1.15"));
  hdr7.SetProtocol (7); 

  hdr8.SetPayloadSize (0);
  hdr8.SetSource (Ipv4Address ("10.10.1.16"));
  hdr8.SetDestination (Ipv4Address ("10.10.1.17"));
  hdr8.SetProtocol (7); 

  hdr9.SetPayloadSize (0);
  hdr9.SetSource (Ipv4Address ("10.10.1.18"));
  hdr9.SetDestination (Ipv4Address ("10.10.1.19"));
  hdr9.SetProtocol (7); 

  hdr10.SetPayloadSize (0);
  hdr10.SetSource (Ipv4Address ("10.10.1.20"));
  hdr10.SetDestination (Ipv4Address ("10.10.1.21"));
  hdr10.SetProtocol (7); 

  AddPacket(queue,hdr);
  AddPacket(queue,hdr);
  item = queue->Dequeue();
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "Packet Enqueued");
  item = queue->Dequeue();
  NS_TEST_EXPECT_MSG_EQ ((item == 0), true, "Packet Dequeued");

  // test 3:
  queue->Initialize();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (1000)), true,
                         "Verify that we can actually set the attribute QueueLimit");

  for(int i=0;i<25;i++)
  {
   AddPacket(queue,hdr);
   AddPacket(queue,hdr2);
   AddPacket(queue,hdr3);
   AddPacket(queue,hdr4);
  }

  QueueDisc::Stats st = queue->GetStats ();

  uint32_t test1 = st.GetNDroppedBytes (BredQueueDisc::FORCED_DROP)+st.GetNDroppedBytes (BredQueueDisc::UNFORCED_DROP)+st.GetNDroppedBytes(QueueDisc::INTERNAL_QUEUE_DROP);
// Second Scenario
  
  queue->Initialize();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (10000)), true,
                         "Verify that we can actually set the attribute QueueLimit");

  for(int i=0;i<25;i++)
  {
   AddPacket(queue,hdr);
   AddPacket(queue,hdr2);
   AddPacket(queue,hdr3);
   AddPacket(queue,hdr4);
  }


  st = queue->GetStats ();
  uint32_t test2 = st.GetNDroppedBytes (BredQueueDisc::FORCED_DROP)+st.GetNDroppedBytes (BredQueueDisc::UNFORCED_DROP)+st.GetNDroppedBytes(QueueDisc::INTERNAL_QUEUE_DROP);
  NS_TEST_EXPECT_MSG_EQ ((test2>test1), true, "Packet Dequeued");


//test 5

  queue->Initialize();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (1000)), true,
                         "Verify that we can actually set the attribute QueueLimit");

  for(int i=0;i<10;i++)
  {
   AddPacket(queue,hdr);
   AddPacket(queue,hdr2);
   AddPacket(queue,hdr3);
   AddPacket(queue,hdr4);
   AddPacket(queue,hdr5);
   AddPacket(queue,hdr6);
   AddPacket(queue,hdr7);
   AddPacket(queue,hdr8);
   AddPacket(queue,hdr9);
   AddPacket(queue,hdr10);
  }


  st = queue->GetStats ();
  
  uint32_t test5 = st.GetNDroppedBytes (BredQueueDisc::FORCED_DROP)+st.GetNDroppedBytes (BredQueueDisc::UNFORCED_DROP)+st.GetNDroppedBytes(QueueDisc::INTERNAL_QUEUE_DROP);
   
  NS_TEST_EXPECT_MSG_EQ ((test5>test1), true, "Packet Dequeued");

  
}


void 
BredQueueDiscTestCase::Enqueue (Ptr<BredQueueDisc> queue, uint32_t size, uint32_t nPkt, bool ecnCapable)
{
  Address dest;
  for (uint32_t i = 0; i < nPkt; i++)
    {
      queue->Enqueue (Create<BredQueueDiscTestItem> (Create<Packet> (size), dest, 0, ecnCapable));
    }
}

void
BredQueueDiscTestCase::DoRun (void)
{
  RunBredTest (StringValue ("QUEUE_DISC_MODE_BYTES"));
  Simulator::Destroy ();

}


static class BredQueueDiscTestSuite : public TestSuite
{
public:
  BredQueueDiscTestSuite ()
    : TestSuite ("balanced-red-queue-disc", UNIT)
  {
    AddTestCase (new BredQueueDiscTestCase (), TestCase::QUICK);
  }
} g_bredQueueDiscTestSuite; ///< the test suite
