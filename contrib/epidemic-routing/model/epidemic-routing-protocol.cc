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

#include "epidemic-routing-protocol.h"
#include <vector>
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/random-variable-stream.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/udp-header.h"
#include <iostream>
#include <algorithm>
#include <functional>
#include "ns3/ipv4-route.h"
#include "ns3/socket.h"
#include "ns3/log.h"

/**
 * \file
 * \ingroup epidemic
 * ns3::Epidemic::RoutingProtocol implementation.
 */

using namespace std;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EpidemicRoutingProtocol");

namespace Epidemic {
NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);

TypeId
RoutingProtocol::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::Epidemic::RoutingProtocol")
          .SetParent<Ipv4RoutingProtocol> ()
          .AddConstructor<RoutingProtocol> ()
          .AddAttribute ("HopCount",
                         "Maximum number of times "
                         "a packet will be flooded.",
                         UintegerValue (64), MakeUintegerAccessor (&RoutingProtocol::m_hopCount),
                         MakeUintegerChecker<uint32_t> ())
          .AddAttribute ("QueueLength",
                         "Maximum number of "
                         "packets that a queue can hold.",
                         UintegerValue (64), MakeUintegerAccessor (&RoutingProtocol::m_maxQueueLen),
                         MakeUintegerChecker<uint32_t> ())
          .AddAttribute ("QueueEntryExpireTime",
                         "Maximum time a packet can live in "
                         "the epidemic queues since it's generated at the source.",
                         TimeValue (Seconds (100)),
                         MakeTimeAccessor (&RoutingProtocol::m_queueEntryExpireTime),
                         MakeTimeChecker ())
          .AddAttribute ("HostRecentPeriod",
                         "Time in seconds for host recent period"
                         ", in which hosts can not re-exchange summary vectors.",
                         TimeValue (Seconds (10)),
                         MakeTimeAccessor (&RoutingProtocol::m_hostRecentPeriod),
                         MakeTimeChecker ())
          .AddAttribute ("BeaconInterval",
                         "Time in seconds after which a "
                         "beacon packet is broadcast.",
                         TimeValue (Seconds (1)),
                         MakeTimeAccessor (&RoutingProtocol::m_beaconInterval), MakeTimeChecker ())
          .AddAttribute ("BeaconRandomness",
                         "Upper bound of the uniform distribution"
                         " random time added to avoid collisions. Measured in milliseconds",
                         UintegerValue (100),
                         MakeUintegerAccessor (&RoutingProtocol::m_beaconMaxJitterMs),
                         MakeUintegerChecker<uint32_t> ())
          .AddTraceSource ("TraceSendPacket", "has epidemic trace SendPacketFromQueue",
                           MakeTraceSourceAccessor (&RoutingProtocol::m_sendPacketTrace),
                           "ns3::Epidemic::RoutingProtocol::SendTracedCallback")
          .AddTraceSource ("TraceupdateDeliveryPredFor", "trace P(a,b) value",
                           MakeTraceSourceAccessor (&RoutingProtocol::m_updateDeliveryPredForTrace),
                           "ns3::Epidemic::RoutingProtocol::SendTracedCallback")
          .AddTraceSource ("TraceupdateTransitivePreds", "substitute P(a,c) value",
                           MakeTraceSourceAccessor (&RoutingProtocol::m_updateTransitivePredsTrace),
                           "ns3::Epidemic::RoutingProtocol::SendTracedCallback")
          .AddTraceSource ("TracesendPacketFromQueue", "substitute P(b, c) value",
                           MakeTraceSourceAccessor (&RoutingProtocol::m_uptdateSendPacketFromQueue),
                           "ns3::Epidemic::RoutingProtocol::SendTracedCallback")
          .AddTraceSource ("TraceRouteInput", "make queuelist in main.cc",
                           MakeTraceSourceAccessor (&RoutingProtocol::m_uptdateRouteInput),
                           "ns3::Epidemic::RoutingProtocol::SendTracedCallback")   
          .AddTraceSource ("TraceGetJudge_Queuelist", "get queuelist in main.cc",
                           MakeTraceSourceAccessor (&RoutingProtocol::m_updateGetJudge_Queuelist),
                           "ns3::Epidemic::RoutingProtocol::SendTracedCallback")
          .AddTraceSource ("Tracepastcontact", "update pastcontact",
                           MakeTraceSourceAccessor (&RoutingProtocol::m_updatepastcontact),
                           "ns3::Epidemic::RoutingProtocol::SendTracedCallback")
          .AddTraceSource ("TraceJudgeFrompastcontact", "update judge from pastcontact",
                           MakeTraceSourceAccessor (&RoutingProtocol::m_JudgeFrompastcontact),
                           "ns3::Epidemic::RoutingProtocol::SendTracedCallback")
  ;
  return tid;
}

RoutingProtocol::RoutingProtocol ()
    : m_hopCount (0),
      m_maxQueueLen (0),
      m_queueEntryExpireTime (Seconds (0)),
      m_beaconInterval (Seconds (0)),
      m_hostRecentPeriod (Seconds (0)),
      m_beaconMaxJitterMs (0),
      m_dataPacketCounter (0),
      m_queue (m_maxQueueLen)
{
  NS_LOG_FUNCTION (this);
}

RoutingProtocol::~RoutingProtocol ()
{
  NS_LOG_FUNCTION (this);
}

void
RoutingProtocol::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_ipv4 = 0;
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::iterator iter = m_socketAddresses.begin ();
       iter != m_socketAddresses.end (); iter++)
    {
      iter->first->Close ();
    }
  m_socketAddresses.clear ();
  Ipv4RoutingProtocol::DoDispose ();
}

void
RoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
  /*
   *  There is no routing table
   */
  *stream->GetStream () << "No Routing table ";
}

void
RoutingProtocol::Start ()
{
  NS_LOG_FUNCTION (this);
  m_queue.SetMaxQueueLen (m_maxQueueLen);
  m_beaconTimer.SetFunction (&RoutingProtocol::SendBeacons, this);
  m_beaconJitter = CreateObject<UniformRandomVariable> ();
  m_beaconJitter->SetAttribute ("Max", DoubleValue (m_beaconMaxJitterMs));
  m_beaconTimer.Schedule (m_beaconInterval + MilliSeconds (m_beaconJitter->GetValue ()));
}

bool
RoutingProtocol::IsHostContactedRecently (Ipv4Address hostID)
{
  NS_LOG_FUNCTION (this << hostID);
  // If host is not in has table, record current time and return false
  HostContactMap::iterator hostID_pair = m_hostContactTime.find (hostID);
  if (hostID_pair == m_hostContactTime.end ())
    {
      m_hostContactTime.insert (std::make_pair (hostID, Now ()));
      return false;
    }

  //if host is in hash table check time is less than the recent_period:
  if (Now () < (hostID_pair->second + m_hostRecentPeriod))
    {
      // it means the host is recently contacted
      return true;
    }
  else
    {
      // update the recent contact value
      hostID_pair->second = Now ();
      // return false since it has exceeded the recent contact period
      return false;
    }
}

void
RoutingProtocol::SendPacket (Ptr<Packet> p, InetSocketAddress addr)
{
  NS_LOG_FUNCTION (this << p << addr);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin ();
       j != m_socketAddresses.end (); ++j)
    {
      Ipv4InterfaceAddress iface = j->second;
      if (iface.GetLocal () == m_mainAddress)
        {
          Ptr<Socket> socket = j->first;
          NS_LOG_LOGIC ("Packet " << p << " is sent to" << addr);
          socket->SendTo (p, 0, addr);
        }
    }
  NS_LOG_FUNCTION (this << *p);
}

void
RoutingProtocol::BroadcastPacket (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin ();
       j != m_socketAddresses.end (); ++j)
    {
      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      // Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
      Ipv4Address destination;
      if (iface.GetMask () == Ipv4Mask::GetOnes ())
        {
          destination = Ipv4Address ("255.255.255.255");
        }
      else
        {
          destination = iface.GetBroadcast ();
        }
      NS_LOG_LOGIC ("m_mainAddress " << m_mainAddress << ", Packet " << p << " is sent to" << destination);
      socket->SendTo (p, 0, InetSocketAddress (destination, EPIDEMIC_PORT));
    }
}

void
RoutingProtocol::SendPacketFromQueue (Ipv4Address dst, QueueEntry queueEntry)
{
  NS_LOG_LOGIC (this << dst << queueEntry.GetPacketID ()); // function to warn
  Ptr<Packet> p = ConstCast<Packet> (queueEntry.GetPacket ());
  UnicastForwardCallback ucb = queueEntry.GetUnicastForwardCallback ();
  Ipv4Header header = queueEntry.GetIpv4Header ();
  /*
   *  Since Epidemic routing has a control mechanism to drop packets based
   *  on hop count, IP TTL dropping mechanism is avoided by incrementing TTL.
   */

  m_uptdateSendPacketFromQueue (dst, p);
  
  NS_LOG_WARN ("(SendPacketFromQueue), " << "Time : " << Simulator::Now ().GetSeconds () << "packet uid is " << p->GetUid() << ", packet size is " << p->GetSize() << ", main address is " << m_mainAddress << ", dst is " << dst);

  NS_LOG_WARN("(SendPacketFromQueue), " << "Time : " << Simulator::Now ().GetSeconds () << ", preds["
              << header.GetDestination () << "] = " << preds[header.GetDestination ()] << ", "
              << "opp_preds2[" << header.GetDestination ()
              << "] = " << opp_preds2[header.GetDestination ()]);

    //if (preds[header.GetDestination ()] < opp_preds2[header.GetDestination ()] || header.GetDestination() == dst)
     // {
      header.SetTtl (header.GetTtl () + 1);
      header.SetPayloadSize (p->GetSize ());
      Ptr<Ipv4Route> rt = Create<Ipv4Route> ();
      rt->SetSource (header.GetSource ()); 
      rt->SetDestination (header.GetDestination ());
      rt->SetGateway (dst);

      if (m_ipv4->GetInterfaceForAddress (m_mainAddress) != -1)
        {
          rt->SetOutputDevice (
              m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (m_mainAddress)));
        }

      Ptr<Packet> copy = p->Copy ();
      /*
    *  The packet will not be sent if:
    *  The forward address is the source address of the packet.
    *  The forward address is the destination address of the packet.
    */
      if (dst != header.GetSource () && !IsMyOwnAddress (header.GetDestination ()))
        {
          ucb (rt, copy, header);
        }
   // }
}

void
RoutingProtocol::SendBeacons ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("m_mainAddress " << m_mainAddress << " is sent beacons");

  Ptr<Packet> packet = Create<Packet> ();
  EpidemicHeader header;
  // This number does not have any effect but it has to be more than
  // 1 to avoid dropping at the receiver
  header.SetHopCount (m_hopCount);
  packet->AddHeader (header);
  TypeHeader tHeader (TypeHeader::BEACON);
  packet->AddHeader (tHeader);
  ControlTag tempTag (ControlTag::CONTROL);
  // Packet tag is added and will be removed before local delivery in
  // RouteInput function
  packet->AddPacketTag (tempTag);

  BroadcastPacket (packet);
  m_beaconTimer.Schedule (m_beaconInterval + MilliSeconds (m_beaconJitter->GetValue ()));
}

uint32_t
RoutingProtocol::FindOutputDeviceForAddress (Ipv4Address dst)
{
  NS_LOG_FUNCTION (this << dst);
  Ptr<Node> mynode = m_ipv4->GetObject<Node> ();
  for (uint32_t i = 0; i < mynode->GetNDevices (); i++)
    {
      Ipv4InterfaceAddress iface =
          m_ipv4->GetAddress (m_ipv4->GetInterfaceForDevice (mynode->GetDevice (i)), 0);
      if (dst.CombineMask (iface.GetMask ()) == iface.GetLocal ().CombineMask (iface.GetMask ()))
        {
          return i;
        }
    }
  return -1;
}

uint32_t
RoutingProtocol::FindLoopbackDevice ()
{
  NS_LOG_FUNCTION (this);
  Ptr<Node> mynode = m_ipv4->GetObject<Node> ();
  for (uint32_t i = 0; i < mynode->GetNDevices (); i++)
    {
      Ipv4InterfaceAddress iface =
          m_ipv4->GetAddress (m_ipv4->GetInterfaceForDevice (mynode->GetDevice (i)), 0);
      if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
        {
          return i;
        }
    }
  return -1;
}

bool
RoutingProtocol::IsMyOwnAddress (Ipv4Address src)
{
  NS_LOG_FUNCTION (this << src);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin ();
       j != m_socketAddresses.end (); ++j)
    {
      Ipv4InterfaceAddress iface = j->second;
      if (src == iface.GetLocal ())
        {
          return true;
        }
    }
  return false;
}

Ptr<Ipv4Route>
RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif,
                              Socket::SocketErrno &sockerr)
{

  NS_LOG_FUNCTION (this << p << header << oif << sockerr);
  NS_LOG_LOGIC (this << "Packet Size" << p->GetSize () << " Packet " << p->GetUid ()
                     << " reached node " << m_mainAddress << " source  " << header.GetSource ()
                     << " going to " << header.GetDestination ());

  if (IsMyOwnAddress (header.GetDestination ()))
    {
      NS_LOG_LOGIC ("Local delivery a packet" << p->GetUid () << " has arrived destination "
                                              << " At node " << m_mainAddress << "  " << header);
      Ptr<Ipv4Route> rt = Create<Ipv4Route> ();
      rt->SetSource (m_mainAddress);
      rt->SetDestination (header.GetDestination ());
      return rt;
    }
  else
    {
      Ptr<Ipv4Route> rt = Create<Ipv4Route> ();
      rt->SetSource (m_mainAddress);
      rt->SetDestination (header.GetDestination ());
      rt->SetGateway (header.GetDestination ());

      if (m_ipv4->GetInterfaceForAddress (m_mainAddress) != -1)
        {
          /*
           *  Control packets generated at this node, are
           *  tagged with ControlTag.
           *  They are removed before local delivery in RouteInput function.
           */
          ControlTag tag;
          p->PeekPacketTag (tag);
          if (tag.GetTagType () == ControlTag::CONTROL)
            {
              /*
               * if the packet is not control, it means a data packet
               * Thus, data packet is supposed to be looped back to
               * store it in the epidemic queue.
               */
              rt->SetOutputDevice (m_ipv4->GetNetDevice (FindLoopbackDevice ()));
            }
          else
            {
              /*
               * if the packet is control packet
               * Thus, find the corresponding output device
               */
              NS_LOG_DEBUG ("Epidemic triggered packets :"
                            << header.GetDestination () << "  found "
                            << FindOutputDeviceForAddress (header.GetDestination ()));
              rt->SetOutputDevice (
                  m_ipv4->GetNetDevice (FindOutputDeviceForAddress (header.GetDestination ())));
            }
        }
      return rt;
    }
}

bool
RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv4Header &header,
                             Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
                             MulticastForwardCallback mcb, LocalDeliverCallback lcb,
                             ErrorCallback ecb)
{
  NS_LOG_FUNCTION (this << header << *p);
  NS_ASSERT (m_ipv4 != 0);
  NS_ASSERT (p != 0);
  // Check if input device supports IP
  NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
  /*
  If there are no interfaces, ignore the packet and return false
  */
  if (m_socketAddresses.empty ())
    {
      NS_LOG_ERROR ("No  interfaces");
      return false;
    }

  if (header.GetTtl () < 1)
    {
      NS_LOG_DEBUG ("TTL expired, Packet is dropped " << p->GetUid ());
      return false;
    }

  if (header.GetProtocol () == 1)
    {
      NS_LOG_DEBUG ("Does not deliver ICMP packets " << p->GetUid ());
      return false;
    }

  /*
   *  Check all the interfaces local addresses for local delivery
   */

  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin ();
       j != m_socketAddresses.end (); ++j)
    {
      Ipv4InterfaceAddress iface = j->second;
      int32_t iif = m_ipv4->GetInterfaceForDevice (idev);
      if (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()) == iif)
        {
          if (header.GetDestination () == iface.GetBroadcast () ||
              header.GetDestination () == m_mainAddress)
            {
              ControlTag tag;
              p->PeekPacketTag (tag);
              Ptr<Packet> local_copy = p->Copy ();
              bool duplicatePacket = false;
              /*
               * If this is a data packet, add it to the epidemic
               * queue in order to avoid
               * receiving duplicates of the same packet.
               */
              if (tag.GetTagType () == ControlTag::NOT_SET)
                {
                  Ptr<Packet> copy = p->Copy ();
                  QueueEntry newEntry (copy, header, ucb, ecb);
                  EpidemicHeader current_Header;
                  copy->PeekHeader (current_Header);
                  newEntry.SetExpireTime (m_queueEntryExpireTime + current_Header.GetTimeStamp ());
                  newEntry.SetPacketID (current_Header.GetPacketID ());
                  EpidemicHeader header;
                  local_copy->RemoveHeader (header);
                  // Try to see the packet has been
                  // delivered i.e. in the epidemic buffer
                  if (m_queue.Find (current_Header.GetPacketID ()).GetPacketID () == 0)
                    {
                      // std::cout << "main_address_1 is " << m_mainAddress << ", " ;
                      if (p->GetSize() >= 200)
                      {
                        m_updateGetJudge_Queuelist (p);
                      }

                      if (!(p->GetSize() >= 200 && judge == true))
                      {
                        if (p->GetSize() >= 200)
                          NS_LOG_LOGIC("(RouteInput) packet " << p->GetUid() << " ,packet size " << p->GetSize() << " is entry");
                        m_queue.Enqueue (newEntry, m_mainAddress);
                      }

                      if (p->GetSize() >= 200)
                      {
                          m_uptdateRouteInput(p);
                      }
                      //m_queue.MyDrop();
                    }
                  else
                    {
                      duplicatePacket = true;
                    }
                }
              /*
              Deliver the packet locally
              */
              if (!duplicatePacket)
                {
                  local_copy->RemovePacketTag (tag);
                  lcb (local_copy, header, iif);
                }
              return true;
            }
        }
    }

  /*
  If the packet does not have an epidemic header,
  create one and attach it to the packet.
  This condition occurs when the packet is originated locally and does not have
  an Epidemic header.
  */

  Ptr<Packet> copy = p->Copy ();
  NS_LOG_LOGIC ("Creating Epidemic packet " << p->GetUid () << " Src " << header.GetSource ()
                                            << " Dest " << header.GetDestination ()
                                            << " Size before" << copy->GetSize ());
  /*
   * The global packet id format: 16bit(Source IP):16bit(source packet counter)
   */
  uint16_t hostID = header.GetSource ().Get () & 0xFFFF;
  m_dataPacketCounter++;
  uint32_t global_packet_ID = hostID;
  global_packet_ID = global_packet_ID << 16 | m_dataPacketCounter;

  // Adding the data packet to the queue
  QueueEntry newEntry (copy, header, ucb, ecb);
  newEntry.SetPacketID (global_packet_ID);

  if (IsMyOwnAddress (header.GetSource ()))
    {
      NS_LOG_DEBUG ("Adding Epidemic packet header " << p->GetUid ());
      //ADD EPIDEMIC HEADER
      EpidemicHeader new_Header;
      new_Header.SetPacketID (global_packet_ID);
      new_Header.SetTimeStamp (Simulator::Now ());
      new_Header.SetHopCount (m_hopCount);
      // If the packet is generated in this node, add the epidemic header
      copy->AddHeader (new_Header);
      // If the packet is generated in this node,
      // make the Expire time start from now + the user specified period
      newEntry.SetExpireTime (m_queueEntryExpireTime + Simulator::Now ());
    }
  else
    {
      // If the packet is generated in another node, read the epidemic header
      EpidemicHeader current_Header;
      copy->RemoveHeader (current_Header);
      if (current_Header.GetHopCount () <= 1 ||
          (current_Header.GetTimeStamp () + m_queueEntryExpireTime) < Simulator::Now ())
        {
          // Exit  the function to not add the packet to the queue
          // since the flood count limit is reached
          NS_LOG_DEBUG ("Exit the function  and not add the "
                        "packet to the queue since the flood count limit is reached");
          return true;
        }
      // If the packet is generated in another node,
      // use the timestamp from the epidemic header
      newEntry.SetExpireTime (m_queueEntryExpireTime + current_Header.GetTimeStamp ());
      // If the packet is generated in another node,
      // use the PacketID from the epidemic header
      newEntry.SetPacketID (current_Header.GetPacketID ());
      //Decrease the packet flood counter
      current_Header.SetHopCount (current_Header.GetHopCount () - 1);
      // Add the updated header
      copy->AddHeader (current_Header);
    }

  // std::cout << "main_address_2 is " << m_mainAddress << ", " ;

  if (p->GetSize() >= 200)
  {
    m_updateGetJudge_Queuelist(p);
  }
  
  if (!(p->GetSize() >= 200 && judge == true))
  {
    if (p->GetSize() >= 200)
      NS_LOG_LOGIC("(RouteInput) packet " << p->GetUid() << " ,packet size " << p->GetSize() << " is entry");
    m_queue.Enqueue (newEntry, m_mainAddress);
  }
    

  if (p->GetSize () >= 100)
    {
      m_uptdateRouteInput (p);
    }
  //m_queue.MyDrop(); // original
  return true;
}

void
RoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_LOG_FUNCTION (this << ipv4);
  m_ipv4 = ipv4;
  Simulator::ScheduleNow (&RoutingProtocol::Start, this);
}

void
RoutingProtocol::NotifyInterfaceUp (uint32_t i)
{
  NS_LOG_FUNCTION (this << i);
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  Ipv4InterfaceAddress iface = l3->GetAddress (i, 0);
  if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
    {
      return;
    }
  if (m_mainAddress == Ipv4Address ())
    {
      m_mainAddress = iface.GetLocal ();
    }

  /*
  Create a socket to be used for epidemic routing port
  */
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (), tid);
  socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvEpidemic, this));
  socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), EPIDEMIC_PORT));
  socket->BindToNetDevice (l3->GetNetDevice (i));
  socket->SetAllowBroadcast (true);
  m_socketAddresses.insert (std::make_pair (socket, iface));
}

void
RoutingProtocol::NotifyInterfaceDown (uint32_t i)
{
  NS_LOG_FUNCTION (this << m_ipv4->GetAddress (i, 0).GetLocal ());
  // Disable layer 2 link state monitoring (if possible)
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  Ptr<NetDevice> dev = l3->GetNetDevice (i);
  // Close socket
  Ptr<Socket> socket = FindSocketWithInterfaceAddress (m_ipv4->GetAddress (i, 0));
  NS_ASSERT (socket);
  socket->Close ();
  m_socketAddresses.erase (socket);
}

void
RoutingProtocol::NotifyAddAddress (uint32_t i, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this << i << address);
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  if (!l3->IsUp (i))
    {
      return;
    }
  if (l3->GetNAddresses (i) == 1)
    {
      Ipv4InterfaceAddress iface = l3->GetAddress (i, 0);
      Ptr<Socket> socket = FindSocketWithInterfaceAddress (iface);
      if (!socket)
        {
          if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
            {
              return;
            }
          // Create a socket to listen only on this interface
          Ptr<Socket> socket =
              Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
          NS_ASSERT (socket != 0);
          socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvEpidemic, this));
          socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), EPIDEMIC_PORT));
          socket->BindToNetDevice (l3->GetNetDevice (i));
          socket->SetAllowBroadcast (true);
          m_socketAddresses.insert (std::make_pair (socket, iface));
        }
    }
  else
    {
      NS_LOG_LOGIC ("Epidemic does not work with more then "
                    "one address per each interface. Ignore added address");
    }
}

void
RoutingProtocol::NotifyRemoveAddress (uint32_t i, Ipv4InterfaceAddress address)
{

  NS_LOG_FUNCTION (this << i << address);
  Ptr<Socket> socket = FindSocketWithInterfaceAddress (address);
  if (socket)
    {
      m_socketAddresses.erase (socket);
      Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
      if (l3->GetNAddresses (i))
        {
          Ipv4InterfaceAddress iface = l3->GetAddress (i, 0);
          // Create a socket to listen only on this interface
          Ptr<Socket> socket =
              Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
          NS_ASSERT (socket != 0);
          socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvEpidemic, this));
          // Bind to any IP address so that broadcasts can be received
          socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), EPIDEMIC_PORT));
          socket->SetAllowBroadcast (true);
          m_socketAddresses.insert (std::make_pair (socket, iface));
        }
    }
  else
    {
      NS_LOG_LOGIC ("Remove address not participating in Epidemic operation");
    }
}

void
RoutingProtocol::SendDisjointPackets (SummaryVectorHeader packet_SMV, Ipv4Address dest)
{
  NS_LOG_FUNCTION (this << dest);
  /*
  This function is used to find send the packets listed in the vector list
  */
  SummaryVectorHeader list = m_queue.FindDisjointPackets (packet_SMV);
  for (std::vector<uint32_t>::iterator i = list.m_packets.begin (); i != list.m_packets.end (); ++i)
    {
      QueueEntry newEntry = m_queue.Find (*i);
      if (newEntry.GetPacket ())
        {
          Simulator::Schedule (Time (0), &RoutingProtocol::SendPacketFromQueue, this, dest,
                               newEntry);
        }
    }
}

Ptr<Socket>
RoutingProtocol::FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr) const
{
  NS_LOG_FUNCTION (this << addr);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin ();
       j != m_socketAddresses.end (); ++j)
    {
      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      if (iface == addr)
        {
          return socket;
        }
    }
  Ptr<Socket> socket;
  return socket;
}

void
RoutingProtocol::SendSummaryVector (Ipv4Address dest, bool firstNode)
{
  NS_LOG_LOGIC (this << m_mainAddress<< ", " << dest << ", " << firstNode);
  NS_LOG_WARN ("(SendSummaryVector), " << Simulator::Now().GetSeconds());
  // Creating the packet
  Ptr<Packet> packet_summary = Create<Packet> ();
  SummaryVectorHeader header_summary = m_queue.GetSummaryVector ();
  packet_summary->AddHeader (header_summary);
  TypeHeader tHeader;
  if (firstNode)
    {
      tHeader.SetMessageType (TypeHeader::REPLY);
    }
  else
    {
      tHeader.SetMessageType (TypeHeader::REPLY_BACK);
    }

  packet_summary->AddHeader (tHeader);
  ControlTag tempTag (ControlTag::CONTROL);
  packet_summary->AddPacketTag (tempTag);
  // Send the summary vector
  NS_LOG_INFO ("Sending the summary vector 2 packet " << header_summary);
  InetSocketAddress addr = InetSocketAddress (dest, EPIDEMIC_PORT);
  SendPacket (packet_summary, addr);
}

/* ORIGINAL
void
RoutingProtocol::RecvEpidemic (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_queue.DropExpiredPackets ();
  Address address;
  Ptr<Packet> packet = socket->RecvFrom (address);
  TypeHeader tHeader (TypeHeader::BEACON);
  packet->RemoveHeader (tHeader);

  InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom (address);
  Ipv4Address sender = inetSourceAddr.GetIpv4 ();
  if (tHeader.GetMessageType () == TypeHeader::BEACON)
    {
      NS_LOG_LOGIC ("Got a beacon from " << sender << " " << packet->GetUid ()
                                         << " " << m_mainAddress);
      std::cout << "Got a beacon from " <<  sender << " " << packet->GetUid ()
                                         << " " << m_mainAddress 
                                         << "at " << Simulator::Now().GetSeconds() << std::endl;
      // Anti-entropy session
      // Check if you have the smaller address and the host has not been
      // contacted recently
      if (m_mainAddress.Get () < sender.Get ()
          && !IsHostContactedRecently (sender))
        {
          SendSummaryVector (sender,true);
        }
    }
  else if (tHeader.GetMessageType () == TypeHeader::REPLY)
    {
      NS_LOG_LOGIC ("Got a A reply from " << sender << " "
                                          << packet->GetUid () << " " << m_mainAddress);
      SummaryVectorHeader packet_SMV;
      packet->RemoveHeader (packet_SMV);
      SendDisjointPackets (packet_SMV, sender);
      SendSummaryVector (sender,false);
    }
  else if (tHeader.GetMessageType () == TypeHeader::REPLY_BACK)
    {
      NS_LOG_LOGIC ("Got a A reply back from " << sender
                                               << " " << packet->GetUid () << " " << m_mainAddress);
      SummaryVectorHeader packet_SMV;
      packet->RemoveHeader (packet_SMV);
      SendDisjointPackets (packet_SMV, sender);

    }
  else
    {
      NS_LOG_LOGIC ("Unknown MessageType packet ");
    }
}
*/

void
RoutingProtocol::RecvEpidemic (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_queue.DropExpiredPackets ();
  Address address;
  Ptr<Packet> packet = socket->RecvFrom (address);
  TypeHeader tHeader (TypeHeader::BEACON);
  packet->RemoveHeader (tHeader);

  InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom (address);
  Ipv4Address sender = inetSourceAddr.GetIpv4 ();

  if (tHeader.GetMessageType () == TypeHeader::BEACON)
    {
      NS_LOG_WARN ("(RecvEpidemic), Got a beacon from, " << sender << ", " << packet->GetUid () << ", " << m_mainAddress);

      // 接触したノードを過去に接触したことのある配列にtrueとして格納する
      //pastcontact[m_mainAddress][sender] = true;
      //pastcontact[sender][m_mainAddress] = true;

      m_updatepastcontact (m_mainAddress, sender, true);

      updateDeliveryPredFor (sender);
      updateTransitivePreds (sender);

      // Anti-entropy session
      // Check if you have the smaller address and the host has not been
      // contacted recently
      if (m_mainAddress.Get () < sender.Get () && !IsHostContactedRecently (sender))
        {
          SendSummaryVector (sender, true);
        }
    }
  else if (tHeader.GetMessageType () == TypeHeader::REPLY)
    {
      NS_LOG_LOGIC ("Got a A reply from " << sender << " " << packet->GetUid () << " "
                                          << m_mainAddress);
        SummaryVectorHeader packet_SMV;
        packet->RemoveHeader (packet_SMV);
        SendDisjointPackets (packet_SMV, sender);
        SendSummaryVector (sender, false);
    }
  else if (tHeader.GetMessageType () == TypeHeader::REPLY_BACK)
    {
      NS_LOG_LOGIC ("Got a A reply back from " << sender << " " << packet->GetUid () << " " << m_mainAddress);
        SummaryVectorHeader packet_SMV;
        packet->RemoveHeader (packet_SMV);
        SendDisjointPackets (packet_SMV, sender);
    }
  else
    {
      NS_LOG_LOGIC ("Unknown MessageType packet ");
    }
}

///original
// olsropp
void
RoutingProtocol::MyDrop (int uid)
{
  m_queue.MyDrop (uid);
}

void
RoutingProtocol::SendJudge (Ptr<const Packet> packet, bool judge)
{
  m_mem[packet->GetUid ()] = judge;
}

// prophet
//Returns the timestamp of the last encouter of with the host or -1 if
//entry for the host doesn't exist.
//@param host The host to look the timestamp for
//@return the last timestamp of encouter with the host
double
RoutingProtocol::getEncTimeFor (Ipv4Address addr)
{
  std::map<Ipv4Address, double>::iterator it;
  it = lastEncounterTime.find (addr);
  if (it == lastEncounterTime.end ())
    {
      return 0;
    }
  else
    {
      return it->second;
    }
}

// Returns the current prediction (P) value for a host or 0 if entry for
// the host doesn't exist.
// @param host The host to look the P for
// @return the current P value
double
RoutingProtocol::getPredFor (Ipv4Address addr)
{
  ageDeliveryPreds ();
  std::map<Ipv4Address, double>::iterator it;
  it = preds.find (addr);
  if (it == preds.end ())
    {
      return 0;
    }
  else
    {
      return it->second;
    }
}

//Returns a map of this router's delivery predictions
//@return a map of this router's delivery predictions
std::map<Ipv4Address, double>
RoutingProtocol::getDeliveryPreds ()
{
  ageDeliveryPreds ();
  return preds;
}

// Updates delivery predictions for a host.
// <CODE>P(a,b) = P(a,b)_old + (1 - P(a,b)_old) * PEnc
// PEnc(intvl) =
//        P_encounter_max * (intvl / I_typ) for 0<= intvl <= I_typ
//        P_encounter_max for intvl > I_typ</CODE>
void
RoutingProtocol::updateDeliveryPredFor (Ipv4Address addr)
{
  double PEnc;
  // 現在の時刻を代入
  double simTime = Simulator::Now ().GetSeconds ();
  // 最終の更新時刻を代入
  double lastEncTime = getEncTimeFor (addr);

  // ProPHET v2
  // 最終の更新時刻が0の時、PEncにPEncMaxを代入
  if (lastEncTime == 0)
    {
      PEnc = PEncMax;
    }
  else
    {
      // 現在時刻と最終更新時刻の差がI_TYP未満の時
      if ((simTime - lastEncTime) < I_TYP)
        {
          // 以下の式をPEncに代入
          PEnc = PEncMax * ((simTime - lastEncTime) / I_TYP);
        }
      else
        {
          // それ以外の時、PEncにPEncMaxを代入
          PEnc = PEncMax;
        }
    }

  // PEnc = 0.75;  // ProPHET

  // P(a,b)_oldをP(a,b)_newに変更する
  double oldValue = getPredFor (addr);
  double newValue = oldValue + (1 - oldValue) * PEnc;
  preds[addr] = newValue;

  // main.ccのmainmapにP(a,b)_newを代入する。(mainmap[m_mainAddress][addr] = newValue)
  m_updateDeliveryPredForTrace (m_mainAddress, addr, newValue);  

  NS_LOG_WARN ("(updateDeliveryPredFor), " << Simulator::Now ().GetSeconds ()
               << ", P(A, B) = P(, " << m_mainAddress << ", " << addr << ", ) = , " << preds[addr]);

  // 最終更新時刻を現在の時刻に変更する
  lastEncounterTime[addr] = simTime;
}

//Updates transitive (A->B->C) delivery predictions.
//<CODE>P(a,c) = P(a,c)_old + (1 - P(a,c)_old) * P(a,b) * P(b,c) * BETA
//</CODE>
//@param host The B host who we just met
void
RoutingProtocol::updateTransitivePreds (Ipv4Address addr)
{
  // P(a,b)を取り出す
  double pForHost = getPredFor (addr);
  // P(b,c)をotherPredsに取り出す
  m_updateTransitivePredsTrace (addr); // P(b,c)

  for (auto itr = opp_preds.begin (); itr != opp_preds.end (); ++itr)
    {
      judgefrompastcontact = false;
      // aとbが接触し、bとcが接触していることを確認する。
      m_JudgeFrompastcontact(m_mainAddress, addr, itr->first);
      // 取り出したopp_predsがmainAddressの宛先到達確率ではないとき
      if (itr->first == m_mainAddress || judgefrompastcontact == false)
        {
          continue;
        }
      
      // ProPHETv2の宛先到達確率の計算を行う。
      double pOld = getPredFor (itr->first); // P(a,c)_old
      double pNew = pForHost * itr->second * DEFAULT_BATA; // P(a,c)_new

      // ProPHET V2 : max (old, new);
      if (pNew > pOld)
        {
          // ローカルの宛先到達確率を格納する配列に代入
          preds[itr->first] = pNew;
            NS_LOG_WARN ("(updateTransitivePreds), " << Simulator::Now ().GetSeconds ()
               << ", P(A, C) = P(, " << m_mainAddress << ", " << itr->first << ", ) =, " << preds[itr->first]);
          // mainAddress, 宛先をindexにして、新しい宛先到達確率をmain.ccに代入
          m_updateDeliveryPredForTrace (m_mainAddress, itr->first, pNew);
        }

      // ProPHETの宛先到達確率の計算を行う
      // ProPHET : P(a,c) = P(a,c)_old + (1 - P(a,c)_old) * P(a,b) * P(b,c) * BETA
      // preds[itr->first] = pOld + (1 - pOld) * pNew;
      // end
    }
}

//Ages all entries in the delivery predictions.
//<CODE>P(a,b) = P(a,b)_old * (GAMMA ^ k)</CODE>, where k is number of
//time units that have elapsed since the last time the metric was aged.
//@see #SECONDS_IN_UNIT_S
void
RoutingProtocol::ageDeliveryPreds ()
{
  // 秒単位で前回の更新時刻と今回の更新時刻の差分を算出
  double timeDiff = (Simulator::Now ().GetSeconds () - lastAgeUpdate) / secondsInTimeUnit;
  // std::cout << "(ageDeliveryPreds) timeDiff is " << timeDiff << std::endl;

  // 前回の更新時刻と今回の更新時刻に差がない場合は終了
  if (timeDiff == 0)
    {
      return;
    }

  /*
  // timeDiffをint型にキャストする
  int temp = (int) timeDiff;

  // 前回の更新時刻と今回の更新時刻に差がない場合は終了
  if (temp == 0)
    {
      return;
    }
  */

  //  GAMMA ^ k を 算出
  double mult = pow (GAMMA, timeDiff);

  // P(a,i)を新しい値に更新する
  for (auto itr = preds.begin (); itr != preds.end (); ++itr)
    {
      preds[itr->first] = itr->second * mult;
    }

  // 最後の更新の時間に今の時間を代入する
  lastAgeUpdate = Simulator::Now ().GetSeconds ();
}

// Set the probability of main.cc to the probability of epidemic-routing.cc.
void
RoutingProtocol::setOpp_preds (std::map<Ipv4Address, double> main_preds)
{
  opp_preds = main_preds;
}

// Set the probability of main.cc to the probability of epidemic-routing.cc.
void
RoutingProtocol::setOpp_preds2 (std::map<Ipv4Address, double> main_preds)
{
  opp_preds2 = main_preds;
}

void RoutingProtocol::GetBool (bool hikijudge)
{
  judge = hikijudge;
}

void RoutingProtocol::JudgeFrompastcontact (bool judge)
{
  judgefrompastcontact = judge;
}

Ipv4Address RoutingProtocol::GetmainAddress ()
{
  return m_mainAddress;
}

} // namespace Epidemic
} //end namespace ns3
