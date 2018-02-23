/*Copyright

NITK Surathkal

Authors : Amit V. Nene

Mentors : Dr. Mohit P. Tahiliani

*/


#ifndef BRED_QUEUE_DISC_H
#define BRED_QUEUE_DISC_H
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
#include "ns3/queue-disc.h"
#include "ns3/nstime.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class TraceContainer;

class BredQueueDisc : public QueueDisc
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief RedQueueDisc Constructor
   *
   * Create a RED queue disc
   */
  BredQueueDisc ();

  /**
   * \brief Destructor
   *
   * Destructor
   */
  virtual ~BredQueueDisc ();

  /**
   * \brief Drop types
   */

  
  enum
  {
    DTYPE_NONE,        //!< Ok, no drop
    DTYPE_FORCED,      //!< A "forced" drop
    DTYPE_UNFORCED,    //!< An "unforced" (random) drop
  };

  /**
   * \brief Enumeration of the modes supported in the class.
   *
   */
  enum QueueDiscMode
  {
    QUEUE_DISC_MODE_PACKETS,     /**< Use number of packets for maximum queue disc size */
    QUEUE_DISC_MODE_BYTES,       /**< Use number of bytes for maximum queue disc size */
  };

/**
   * \brief Set the operating mode of this queue disc.
   *
   * \param mode The operating mode of this queue disc.
   */
  void SetMode (QueueDiscMode mode);

  /**
   * \brief Get the operating mode of this queue disc.
   *
   * \returns The operating mode of this queue disc.
   */
  QueueDiscMode GetMode (void);

  /**
   * \brief Get the current value of the queue in bytes or packets.
   *
   * \returns The queue size in bytes or packets.
   */
  uint32_t GetQueueSize (void);

  /**
  * \brief Set the limit of the queue.
  *
  * \param lim The limit in bytes or packets.
  */
  void SetQueueLimit (uint32_t lim);

  /*
  Set the value of Wm

  The Window size
  */

  void SetWm (uint32_t Wm);

  /*
  Set alpha
  */

  void SetAlpha (double a);

  /*
  Set beta
  */

  void SetBeta (double b);

  int64_t AssignStreams (int64_t stream);

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose (void);

private:
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void) const;
  virtual bool CheckConfig (void);

  /**
   * \brief Initialize the queue parameters.
   *
   * Note: if the link bandwidth changes in the course of the
   * simulation, the bandwidth-dependent RED parameters do not change.
   * This should be fixed, but it would require some extra parameters,
   * and didn't seem worth the trouble...
   */

  virtual void InitializeParams (void);
public:
  static constexpr const char* UNFORCED_DROP = "Unforced drop";  
  static constexpr const char* FORCED_DROP = "Forced drop";      
  // Reasons for marking packets
  static constexpr const char* UNFORCED_MARK = "Unforced mark";  
  static constexpr const char* FORCED_MARK = "Forced mark";      


  bool m_useEcn;                    //!< True if ECN is used (packets are marked instead of being dropped)
  QueueDiscMode m_mode;             //!< Mode (Bytes or packets)
  uint32_t m_meanPktSize;           //!< Avg pkt size
  uint32_t m_idlePktSize;           //!< Avg pkt size used during idle times
  bool m_useHardDrop;               //!< True if packets are always dropped above Buffer Size

  double m_l1;                      // Number of Packets in buffer after which packet is dropped with probability p1
  double m_l2;                      // Number of Packets in buffer after which packet is dropped with probability p2
  double m_p1;                      // Probability of dropping the packet after l1 packets
  double m_p2;                      // Probability of dropping the packet after l2 packets
  uint32_t m_Wm;                    // Window Size
  double m_wn;
  uint32_t m_Nactive;               // Number of active flows
  uint32_t m_Nestimate;
  uint32_t m_queueLimit;                     // Buffer size
  uint32_t m_N;                     // Number of Flows
  double m_AlphaBred;              //
  double m_BetaBred;
  double m_ptc;                     //!< packet time constant in packets/second
  uint32_t m_idle;                  //!< 0/1 idle status

  DataRate m_linkBandwidth;	   //!< Link bandwidth

  Time m_idleTime;                  //!< Start of current idle period

  Ptr<UniformRandomVariable> m_uv;          //!< rng stream
};

};         // namespace ns3

        #endif // BRED_QUEUE_DISC_H
