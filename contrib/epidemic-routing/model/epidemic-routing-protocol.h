/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Mohammed J.F. Alenazi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mohammed J.F. Alenazi  <malenazi@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported by King Saud University and
 * the ITTC at The University of Kansas.
 */

#ifndef EPIDEMIC_ROUTING_PROTOCOL_H
#define EPIDEMIC_ROUTING_PROTOCOL_H

#include "epidemic-packet-queue.h"
#include "epidemic-packet.h"
#include "ns3/random-variable-stream.h"
#include "epidemic-tag.h"
#include <vector>
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/node.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/timer.h"
#include <iostream>
#include <algorithm>
#include <functional>
#include "ns3/ipv4-route.h"
#include "ns3/socket.h"
#include "ns3/log.h"

#include "ns3/olsr-helper.h"

/**
 * \defgroup epidemic Epidemic Routing
 *
 * This section documents the API of the ns-3 epidemic module. For a
 * generic functional description, please refer to the ns-3 model
 * library document.
 */


namespace ns3 {
namespace Epidemic {
/**
 * \ingroup epidemic
 * \brief Epidemic routing protocol
 *  This implementation is based on the paper titled 'Epidemic Routing for
 *  Partially-Connected Ad Hoc Networks'
 *  In the original paper, the implementation was on top of
 *  the Internet MANET Encapsulation Protocol (IMEP) layer, which
 *  is  responsible for notifying the epidemic agent when a new node
 *  comes into radio range. However, since IMEP is not implemented in ns-3
 *  , a beacon mechanism is added to the implementation.
 *  Some functions are adapted from the implementation AODV.
 */
class RoutingProtocol : public Ipv4RoutingProtocol
{
public:
  /**
   *  Get the registered TypeId for this class.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  /// Transport Port for MANET routing protocols ports, based on \RFC{5498}
  static const uint32_t EPIDEMIC_PORT = 269;
  /// c-tor
  RoutingProtocol ();
  /** Dummy destructor, see DoDispose. */
  virtual  ~RoutingProtocol ();
  /** Destructor implementation */
  virtual void  DoDispose ();
  // Inherited methods:
  Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header,
                              Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
  bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header,
                   Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
                   MulticastForwardCallback mcb,
                   LocalDeliverCallback lcb, ErrorCallback ecb);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;
  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface,
                                 Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface,
                                    Ipv4InterfaceAddress address);
  virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  /// original
  void MyDrop(int uid);
  void SendJudge(Ptr<const Packet> packet, bool judge);
  void setOpp_preds(std::map<Ipv4Address, double> main_preds);
  void setOpp_preds2(std::map<Ipv4Address, double> main_preds);
  void GetBool(bool hikijudge);
  void JudgeFrompastcontact (bool judge);
  Ipv4Address GetmainAddress ();

private:
  /// Main IP address for the current node
  Ipv4Address m_mainAddress;
  /// Number of times a packet is resent
  uint32_t m_hopCount;
  /// Maximum number of packets a queue can hold
  uint32_t m_maxQueueLen;
  /// Time in seconds after which the packet will expire in the queue
  Time m_queueEntryExpireTime;
  /// Time in seconds for sending periodic beacon packets
  Time m_beaconInterval;
  /// Time in seconds for host recent period, in which hosts can not
  // re-exchange summary vectors
  Time m_hostRecentPeriod;
  /// Upper bound of the uniform distribution random time added
  // to avoid collisions. Measured in milliseconds
  uint32_t m_beaconMaxJitterMs;
  /// Local counter for data packets
  uint16_t m_dataPacketCounter;
  /// A pointer to the Ipv4 for the current node
  Ptr<Ipv4> m_ipv4;
  /// A map between opened sockets and IP addresses
  std::map<Ptr<Socket>, Ipv4InterfaceAddress> m_socketAddresses;
  /// queue associated with a node
  PacketQueue m_queue;
  /// timer for sending beacons
  Timer m_beaconTimer;
  /// uniform random variable to be added to beacon intervals
  // to avoid collisions
  Ptr<UniformRandomVariable> m_beaconJitter;
  ///  Type to connect a host address to recent contact time value
  typedef std::map<Ipv4Address, Time> HostContactMap;
  /// Pair representing host address and time value
  typedef HostContactMap::value_type   HostContactMapPair;
  /// Hash table to store recent contact time for nodes
  HostContactMap m_hostContactTime;

  /// original
  // olsropp
  std::map<int, bool> m_mem;
  // prophet
  double PEncMax = 0.5;
  double I_TYP = 1800;
  double DEFAULT_BATA = 0.9;
  double GAMMA = 0.999885791;
  std::map<Ipv4Address, double> preds;
  std::map<Ipv4Address, double> opp_preds; // using in recvepidemic
  std::map<Ipv4Address, double> opp_preds2; // using in sendpacketfromqueue
  /// 隣接ノードと最後に出会った時間を保存する配列
  std::map<Ipv4Address, double> lastEncounterTime;
  /// 過去に接触したノードを保存する配列
  std::map<Ipv4Address, std::map<Ipv4Address, bool>> pastcontact;
  
  double lastAgeUpdate;
  double secondsInTimeUnit = 30.0;

  /// 隣接ノードと接触した時に自ノードの宛先到達確率を更新する。更新した宛先到達確率は配列(preds)に代入する。
  void updateDeliveryPredFor (Ipv4Address addr);
  /// 時間経過で自ノードの宛先到達確率を更新し、更新した宛先到達確率を配列(preds)に代入する
  void ageDeliveryPreds();
  /// P(a, c)を算出する関数 (a->b->c)
  void updateTransitivePreds(Ipv4Address addr);

  /// lastEncounterTimeから最後に隣接ノードと出会った時間を呼び出す関数
  double getEncTimeFor(Ipv4Address addr);
  /// 特定の宛先到達確率をpredsから取り出す関数 (m_mainAddress:自ノード, addr:宛先)
  double getPredFor(Ipv4Address addr);
  /// 自ノードの宛先到達確率をageDeliveryPredsを用いて算出し配列(preds)に代入する。その後、配列をreturnする。
  std::map<Ipv4Address, double> getDeliveryPreds();


  bool judge;
  bool judgefrompastcontact;

  /// Start protocol operation
  /// Invoked by SetIpv4()
  void Start ();
  /**
   * \brief Captures incoming Epidemic packets.
   * \param socket Epidemic socket.
   */
  void RecvEpidemic (Ptr<Socket> socket);
  /**
   * \brief Sends disjoint packets given a vector containing
   *   the other nodes Packet IDs and its IP address
   * \param packet_SMV vector containing the other nodes Packet IDs
   * \param dest destination address
   */
  void SendDisjointPackets (
    SummaryVectorHeader packet_SMV, Ipv4Address dest);
  /// Function to send beacons periodically
  void SendBeacons ();
  /**
   * \brief Finding output device for a given IP address.
   * \param dst the given IP address.
   * \returns the output device ID.
   */
  uint32_t FindOutputDeviceForAddress ( Ipv4Address  dst);
  /// \returns the loopback device for current node
  uint32_t FindLoopbackDevice ();
  /**
   * \brief Sending a packet to a given IP address.
   * \param p the packet to be sent.
   * \param addr the destination IP address.
   */
  void SendPacket (Ptr<Packet> p,InetSocketAddress addr);
  /**
   * \brief Check i a given IP address is associated with the current node.
   * \param src the IP address to be checked.
   * \returns true if an address belongs to the current node
   */
  bool IsMyOwnAddress (Ipv4Address src);
  /**
   * \brief Broadcast a given packet.
   * \param p the packet to be broadcasted.
   */
  void BroadcastPacket (Ptr<Packet> p);
  /**
   * \brief Send summary vector
   * \param dest destination address
   * \param firstNode check the anti-entropy session in the Epidemic paper
   *    \c true send a summary vector with reply header
   *    \c false send a summary vector with reply back header
   */
  void SendSummaryVector (Ipv4Address dest,bool firstNode);
  /**
   * \brief Finding the corresponding socket for the given interface.
   * \returns socket for the given interface
   */
  Ptr<Socket> FindSocketWithInterfaceAddress (
    Ipv4InterfaceAddress iface) const;
  /**
   * \brief Send a a packet from a queue via \p queueEntry to address \p dst.
   * \param dst the destination IP address.
   * \param queueEntry the queue entry containing the packet.
   */
  void SendPacketFromQueue (Ipv4Address dst,QueueEntry queueEntry);
  /// \returns true if \p hostID has been contacted recently
  /**
   * \brief Check if a given node has been contacted recently .
   * \returns true if hostID has been contacted recently
   */
  bool IsHostContactedRecently (Ipv4Address hostID);

  //original
  TracedCallback <Ptr<const Packet>, Ipv4Address> m_sendPacketTrace;

  typedef void (*SendTracedCallback)
    (Ptr<const Packet> packet, Ipv4Address dest);

  TracedCallback <Ipv4Address, Ipv4Address, double> m_updateDeliveryPredForTrace;

  typedef void (*TraceupdateDeliveryPredFor)
    (Ipv4Address mainAddress, Ipv4Address addr, double newValue);

  TracedCallback <Ipv4Address> m_updateTransitivePredsTrace;
  
  typedef void (*TraceupdateTransitivePreds)
    (Ipv4Address addr);

  TracedCallback < Ipv4Address, Ptr<const Packet> > m_uptdateSendPacketFromQueue;

  typedef void (*TracesendPacketFromQueue)
    (Ipv4Address dst, Ptr<const Packet> packet);

  TracedCallback <Ptr<const Packet>> m_uptdateRouteInput;

  typedef void (*TraceRouteInput)
    (Ptr<const Packet>);
  
  TracedCallback <Ptr<const Packet>> m_updateGetJudge_Queuelist;

  typedef void (*TraceGetJudge_Queuelist)
    (Ptr<const Packet>);

  TracedCallback <Ipv4Address, Ipv4Address, bool> m_updatepastcontact;

  typedef void (*Tracepastcontact)
    (Ipv4Address address, Ipv4Address address2, bool judge);

  TracedCallback <Ipv4Address, Ipv4Address, Ipv4Address> m_JudgeFrompastcontact;

  typedef void (*TraceJudgeFrompastcontact)
    (Ipv4Address address, Ipv4Address address2, Ipv4Address address3);

};
} //end namespace epidemic
} //end namespace ns3
#endif
