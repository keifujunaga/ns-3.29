/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/regular-wifi-mac.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/epidemic-helper.h"
#include <ns3/buildings-module.h>
#include "ns3/config-store-module.h"
#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"

// NetAnim
#include "ns3/netanim-module.h"

// Others
#include <boost/algorithm/string.hpp>
#include <vector>
#include <random>
#include <fstream>
#include <string>
#include <array>

// Epidemic
#include "ns3/epidemic-routing-protocol.h"
#include "ns3/epidemic-packet-queue.h"

// OLSR
#include "ns3/olsr-routing-protocol.h"

#define R 100
#define DRECV 4

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("test-olsropp");

NodeContainer olsrContainer;
NodeContainer dtnContainer;
NetDeviceContainer devices;
NetDeviceContainer devices2;
Ipv4InterfaceContainer interfaces;
Ipv4InterfaceContainer interfaces2;
OlsrHelper olsrh;
EpidemicHelper epidemic;
Ipv4ListRoutingHelper list;

// Parameter
std::string phyMode ("DsssRate1Mbps");
std::string rate = "20.48kbps"; // 1.024kbps
std::string rtslimit = "1500";
uint32_t packetSize = 2048; // 2048
double appTotalTime = 125; // simulator stop time
double appDataStart = 0.0;
double appDataEnd = 125;
int numberOfueNodes = 5;
int numPackets = 1;
uint16_t sinkPort = 6; // use the same for all apps
int RECV = DRECV;
int SENDER = 0;

// From the UID and the sender in tag, check whether the source is sending packet with a specific UID or not.
std::map<int, bool> mem;
// An array that stores the destination arrival probability of prophet (mainaddress, neighboraddress, prediction)
std::map<Ipv4Address, std::map<Ipv4Address, double>> mainmap;
// Array for converting OLSR address to DTN address (OLSR address, DTN address)
std::map<Ipv4Address, Ipv4Address> convarray;

std::map<int, Ipv4Address> convarray_node_olsraddr;
std::map<Ipv4Address, int> convarray_olsraddr_node;
std::map<int, Ipv4Address> convarray_node_dtnaddr;
std::map<Ipv4Address, int> convarray_dtnaddr_node;

std::map<int, std::map<int, bool>> queuelist;

std::map<Ipv4Address, std::map<Ipv4Address, bool>> main_pastcontact;

/**
 *  prototype sengen
 */

// TRACE_SOURCE (FIRST:OLSROPP, SECOND:PROPHET, THIRD:PROPOSAL)

/// Ipv4L3Protocol/Rx : Switching from DTN to OLSR at relay node
void Rx (std::string context, Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t hate);

/// Ipv4L3Protocol/Drop : Swiching from OLSR to DTN when packet is dropped
void Drop (std::string context, const Ipv4Header &, Ptr<const Packet> packet, Ipv4L3Protocol::DropReason reason, Ptr<Ipv4> ipv4, uint32_t interface); 

/// Substitute as P(a, b) in mainmap
void TraceupdateDeliveryPredFor (std::string context, Ipv4Address mainAddress, Ipv4Address addr,
                                 double newValue);
/// Substitute as P(b, i) from mainmap
void TraceupdateTransitivePreds (std::string context, Ipv4Address addr);
/// Substitute as P(b, c) from mainmap !!!!<opp_preds2 iranai setu>!!!!
void TraceSendPacketFromQueue (std::string context, Ipv4Address dst, Ptr<const Packet>);

/// Substitute the destination arrival probability stored in mainmap into the OLSR routing table
void TraceAddEntry (std::string context, Ipv4Address mainAddress, Ipv4Address dest);

/// 過去に接触したことのあるノードを管理する配列を管理する
void ManagingPastcontact (Ipv4Address m_mainAddress, Ipv4Address sender);


// OLSR_OPP

///  Decide to send on DTN or OLSR at first
void olsr_or_dtn (int sender);
/// When the node using DTN completes the OLSR route construction to the destination, it changes from DTN to OLSR.
void dtn_to_olsr (int sender, int recv, int uid);
/// Send a packet using the olsr application.
void olsr_testApp (int recv, int sender, int mode, int uid);
/// Send a packet using the DTN application.
void dtn_testApp (int recv, int sender, int mode, int uid);

// PROPOSAL

/// Substitute destination arrival probability of ProPHET in mainmap of main.cc
void test_olsrroutingtable ();
/// Display routing table of OLSR with destination arrival probability
void test_olsrroutingtable2 ();
/// Create a convarray that changes the OLSR IP address to the DTN IP address.
void makeconvarray ();
/// ノードがどのパケットを保存しているか管理する
void TraceRouteInput(std::string context, Ptr<const Packet> packet);
/// 冗長したパケットを保存しないようにする
void GetJudge_Queuelist(std::string context, Ptr<const Packet> packet);
/// ノードの宛先到達確率を抜き出す
std::map<Ipv4Address, double> ExtractIpPred (Ptr<Node> node);
/// 提案手法の実行を行う関数
void do_proposal (int sender, int uid, int mode);

/*
* Ipv4TestTag Class
*/

class IPv4TestTag : public Tag
{
private:
  uint64_t token;

public:
  static TypeId
  GetTypeId ()
  {
    static TypeId tid =
        TypeId ("ns3::IPv4TestTag").SetParent<Tag> ().AddConstructor<IPv4TestTag> ();
    return tid;
  }
  virtual TypeId
  GetInstanceTypeId () const
  {
    return GetTypeId ();
  }
  virtual uint32_t
  GetSerializedSize () const
  {
    return sizeof (token);
  }
  virtual void
  Serialize (TagBuffer buffer) const
  {
    buffer.WriteU64 (token);
  }
  virtual void
  Deserialize (TagBuffer buffer)
  {
    token = buffer.ReadU64 ();
  }
  virtual void
  Print (std::ostream &os) const
  {
    os << "token=" << token;
  }
  void
  SetToken (uint64_t token)
  {
    this->token = token;
  }
  uint64_t
  GetToken ()
  {
    return token;
  }
};

/*
*     MyApp Class
*/

class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  void Setup (Ptr<Socket> socket, Address address, uint32_t senderAdress, uint32_t receiverAddress,
              uint32_t packetSize, uint32_t nPackets, uint32_t mode, int uid, DataRate dataRate);
  void ChangeRate (DataRate newrate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket> m_socket;
  Address m_peer;
  uint32_t m_packetSize;
  uint32_t m_nPackets;
  DataRate m_dataRate;
  EventId m_sendEvent;
  bool m_running;
  uint32_t m_packetsSent;
  uint32_t m_sender;
  uint32_t m_receiver;
  uint32_t m_mode;
  int m_uid;
};

MyApp::MyApp ()
    : m_socket (0),
      m_peer (),
      m_packetSize (0),
      m_nPackets (0),
      m_dataRate (0),
      m_sendEvent (),
      m_running (false),
      m_packetsSent (0),
      m_sender (0),
      m_receiver (0),
      m_mode (0),
      m_uid (-1)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t sender, uint32_t receiver,
              uint32_t packetSize, uint32_t nPackets, uint32_t mode, int uid, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_sender = sender;
  m_receiver = receiver;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_mode = mode;
  m_uid = uid;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  //if (mem[m_uid * 10 + m_sender] == false) // if this node has never sent a packet
    //{
      IPv4TestTag tag;
      Ptr<Packet> packet = Create<Packet> (m_packetSize);

      if (m_uid == -1) // This packet is generated by sender
        {
          NS_LOG_INFO("(MyApp::SendPacket) Time:" << Simulator::Now().GetSeconds() << ", sender:" << m_sender << " send packet " << packet->GetUid() << " is started");
          tag.SetToken (packet->GetUid () * 10000 + m_receiver * 10 + m_mode);
          packet->AddPacketTag (tag);
          m_socket->Send (packet);
          mem[m_uid * 10 + m_sender] = true;
        }
      else // This packet is generated by intermediary node
        {
          tag.SetToken (m_uid * 10000 + m_receiver * 10 + m_mode);
          packet->AddPacketTag (tag);
          m_socket->Send (packet);
          mem[m_uid * 10 + m_sender] = true;
        }
    //}

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

void
MyApp::ChangeRate (DataRate newrate)
{
  m_dataRate = newrate;
  return;
}

void
IncRate (Ptr<MyApp> app, DataRate rate)
{
  app->ChangeRate (rate);
  return;
}

 void 
 CourseChange (std::string context, Ptr<const MobilityModel> position)
 {
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo
  Vector pos = position->GetPosition ();
  NS_LOG_LOGIC (kym << pos);
  //NS_LOG_INFO(Simulator::Now () << ", kym=" << kym << ", pos=" << position << ", x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z);
 }

/**
 *  PROPHET
 */

void
TraceupdateDeliveryPredFor (std::string context, Ipv4Address mainAddress, Ipv4Address addr,
                            double newValue)
{
  // std::cout << "aaa" << std::endl;
  mainmap[mainAddress][addr] = newValue;
}

void
TraceupdateTransitivePreds (std::string context, Ipv4Address addr)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  if (kym >= numberOfueNodes)
    kym = kym - numberOfueNodes;

  epidemic.setOpp_preds (dtnContainer, kym, mainmap[addr]);
}

void
TraceAddEntry (std::string context, Ipv4Address mainAddress, Ipv4Address dest)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  //std::cout << "(TraceAddEntry) mainmap[convarray[" << dest << "]][Ipv4Address(192.168.1.5)] = " << mainmap[convarray[dest]][Ipv4Address("192.168.2.5")] << std::endl;

  olsrh.setpred (olsrContainer, kym,
                 mainmap[convarray[dest]]);
}

void
TraceSendPacketFromQueue (std::string context, Ipv4Address dst, Ptr<const Packet> packet)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  if (kym >= numberOfueNodes)
    kym = kym - numberOfueNodes;

  epidemic.setOpp_preds2 (dtnContainer, kym, mainmap[dst]);

  IPv4TestTag tagCopy;
  packet->PeekPacketTag (tagCopy);
  int token = tagCopy.GetToken ();
  int uid = token / 10000;
  do_proposal(kym, uid, 1);
}


/**
 * PROPOSAL
 */

void 
Tracepastcontact (std::string context, Ipv4Address address, Ipv4Address address2, bool judge)
{
  main_pastcontact[address][address2] = judge;
  main_pastcontact[address2][address] = judge;
}

void 
TraceJudgeFrompastcontact (std::string context, Ipv4Address address, Ipv4Address address2, Ipv4Address address3)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  if (main_pastcontact[address][address2] == true && main_pastcontact[address2][address3] == true)
  {
    //std::cout << "main_pastcontact[" << address << "][" << address2 << "] == true && main_pastcontact[" << address2 << "][" << address3 << "] == true " << std::endl; 
    Ptr<Node> node = dtnContainer.Get(kym-numberOfueNodes);
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
    Ptr<Epidemic::RoutingProtocol> epi = DynamicCast<Epidemic::RoutingProtocol> (proto);
    epi->JudgeFrompastcontact (true);
  }
}

void 
TraceRouteInput(std::string context, Ptr<const Packet> packet)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  kym = kym - numberOfueNodes;

  IPv4TestTag tagCopy;
  packet->PeekPacketTag (tagCopy);

  int token = tagCopy.GetToken ();

  int uid = token / 10000;
  //int temp = token - uid * 10000;
  //int recv = temp / 10;
  //int mode = temp - recv * 10;

  //NS_LOG_INFO("(TraceRouteInput) kym is " << kym << ", uid is " << uid);

  queuelist[kym][uid] = true;
}


void 
GetJudge_Queuelist(std::string context, Ptr<const Packet> packet)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  kym = kym - numberOfueNodes;

  IPv4TestTag tagCopy;
  packet->PeekPacketTag (tagCopy);

  int token = tagCopy.GetToken ();

  int uid = token / 10000;
  //int temp = token - uid * 10000;
  //int recv = temp / 10;
  //int mode = temp - recv * 10;

  bool judge;

  if (queuelist[kym][uid] == true)
    judge = true; 
  else
    judge = false;  

  Ptr<Node> node = dtnContainer.Get(kym);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
  Ptr<Epidemic::RoutingProtocol> epi = DynamicCast<Epidemic::RoutingProtocol> (proto);
  epi->GetBool(judge);
  
  //NS_LOG_INFO("(TraceGetJudge_Queuelist) kym is " << kym << ", uid is " << uid << ", packet size is " << packet->GetSize() << ", judge is " << judge );
}


std::map<Ipv4Address, double>
ExtractIpPred (Ptr<Node> node)
{
  std::map <Ipv4Address, double> ipandpred;
  
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); 
  NS_ASSERT_MSG (ipv4, "Ipv4 not installed on node");
  Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
  NS_ASSERT_MSG (proto, "Ipv4 routing not installed on node");
  Ptr<olsr::RoutingProtocol> olsr = DynamicCast<olsr::RoutingProtocol> (proto);

  if (olsr)
    {
      std::vector<olsr::RoutingTableEntry> table = olsr->GetRoutingTableEntries ();
      for (uint32_t i = 0; i < table.size (); ++i)
        {
          olsr::RoutingTableEntry route = table[i];
          //std::cout << "dest is " << route.destAddr << ", pred is " << route.pred << ", interface is " << route.interface << std::endl;
          ipandpred[route.destAddr] = route.preds[convarray_node_dtnaddr[RECV]];
        }
    }
  // Olsr may also be in a list
  Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (proto);
  if (list)
    {
      int16_t priority;
      Ptr<Ipv4RoutingProtocol> listProto;
      Ptr<olsr::RoutingProtocol> listOlsr;
      for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
        {
          listProto = list->GetRoutingProtocol (i, priority);
          listOlsr = DynamicCast<olsr::RoutingProtocol> (listProto);
          if (listOlsr)
            {
              std::vector<olsr::RoutingTableEntry> table2 = listOlsr->GetRoutingTableEntries ();
              for (uint32_t i = 0; i < table2.size (); ++i)
                {
                  olsr::RoutingTableEntry route2 = table2[i];
                  /*
                  std::cout << "dest is " << route2.destAddr << ", pred is " << route2.pred << ", interface is " << route2.interface << std::endl;
                  */
                  ipandpred[route2.destAddr] = route2.preds[convarray_node_dtnaddr[RECV]];
                  //std::cout << "(ExtractIpPred) Time: " << Simulator::Now().GetSeconds()<< ", ipandpred[" << route2.destAddr << " = route2.pred = " << ipandpred[route2.destAddr] << std::endl;
                  //NS_LOG_INFO("(ExtractIpPred) Time: " << Simulator::Now().GetSeconds()<< ", ipandpred[" << route2.destAddr << " = route2.pred = " << ipandpred[route2.destAddr]);
                }
            }
        }
    }
    return ipandpred;
}

void
do_proposal (int sender, int uid, int mode)
{
  //NS_LOG_INFO("(do_proposal) Time: " << Simulator::Now().GetSeconds() << " do_proposal()is started");

  mode = 1;
  std::map<Ipv4Address, double> ipandpred;

  // ノードが宛先ではない時
  if (sender != RECV)
    {
      // 経路表のIPアドレスと宛先到達確率を配列として抜き出す
      ipandpred = ExtractIpPred (NodeList::GetNode (sender));

      // 自分の宛先到達確率も抜き出す
      double s_pred = mainmap[convarray[convarray_node_olsraddr[sender]]][convarray[convarray_node_olsraddr[RECV]]];

      // 経路表の宛先到達確率 > 自分の宛先到達確率となる、IPアドレスを持つノードを抽出する
      for (auto i = ipandpred.begin (); i != ipandpred.end (); ++i)
        {
          if (s_pred < i->second)
            {
              // 宛先到達確率が高いノードにOLSRを用いてパケットを送信する。この時、modeは1のまま送信する。
              olsr_testApp (convarray_olsraddr_node[i->first], sender, mode, uid);
            }
        }
      if (queuelist[sender][uid] == false)
        {
          // DTNでパケットを送信する
          dtn_testApp (RECV, sender, mode, uid);
        }
    }
}


/// ArpCache
// ArpCache/Drop
void
ArpDrop (std::string context, Ptr<const Packet> packet)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  IPv4TestTag tagCopy;
  packet->PeekPacketTag (tagCopy);

  int token = tagCopy.GetToken ();

  int uid = token / 10000;
  int temp = token - uid * 10000;
  int recv = temp / 10;
  int mode = temp - recv * 10;

  if (mode == 0 && kym < numberOfueNodes &&
      packet->GetSize () >= 2048) // OLSRモードでパケット損失した時DTNモードに変更する
    {
      //NS_LOG_INFO("(ArpCache/Drop) is started at " << kym << " due to drop (" << packet->GetUid () << ", " << packet->GetSize () << ")");
      do_proposal (kym, uid, mode);
    }
}

/// PacketSink/Rx
//  Switching between DTN and OLSR at relay node
void
RxPS (std::string context, Ptr<const Packet> packet, const Address &)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  IPv4TestTag tagCopy;
  packet->PeekPacketTag (tagCopy);

  int token = tagCopy.GetToken ();

  int uid = token / 10000;
  int temp = token - uid * 10000;
  int recv = temp / 10;
  int mode = temp - recv * 10;

  std::map<Ipv4Address, double> ipandpred;

  if (mode == 1)
    {
      if (kym >= numberOfueNodes)
        {
          kym = kym - numberOfueNodes;
        }

      do_proposal (kym, uid, mode);

      if (kym == RECV)
        {
          NS_LOG_INFO ("(PacketSink/Rx) "
                       << "Time:" << Simulator::Now ().GetSeconds () << ", "
                       << "Success At "
                       << "packet->GetUid(): " << packet->GetUid () << ", "
                       << "uid:" << uid << ", "
                       << "recv:" << kym << ", "
                       << "mode:" << mode);
        }
    }
}

/// Ipv4L3Protocol/Rx
// Switching between DTN and OLSR at relay node
void
Rx (std::string context, Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t hate)
{
  if (packet->GetSize () >= 2048)
    {
      std::string delim ("/");
      std::list<std::string> list_string;
      boost::split (list_string, context, boost::is_any_of (delim));
      auto itr = list_string.begin ();
      ++itr;
      ++itr;
      int kym = std::stoi (*itr); // NodeNo
      // std::cout << "(Ipv4L3Protocol) recv is " << kym <<  std::endl;

      IPv4TestTag tagCopy;
      packet->PeekPacketTag (tagCopy);

      int token = tagCopy.GetToken ();

      int uid = token / 10000;
      int temp = token - uid * 10000;
      int recv = temp / 10;
      int mode = temp - recv * 10;

      if (mode == 1)
      {
        if (kym >= numberOfueNodes){
          kym = kym - numberOfueNodes;
          if (kym < RECV) // if node is relay node try to connect to OLSR
            {
              int time = Simulator::Now ().GetSeconds ();
              int endtime = (int) appDataEnd - time;
              for (int i = 0; i < endtime; i++) 
                {
                  Simulator::Schedule (Seconds (i), &dtn_to_olsr, kym, RECV, uid);
                }
            }
        }
      } 
      /*
      NS_LOG_INFO ("(Ipv4L3Protocol/Rx) "
                       << "Time:" << Simulator::Now ().GetSeconds () << ", "
                       << "Success At "
                       << "packet->GetUid(): " << packet->GetUid () << ", "
                       << "uid:" << uid << ", "
                       << "recv:" << kym << ", "
                       << "mode:" << mode);
      */
    }
}

/*
*   execute application
*/


void
dtn_to_olsr (int sender, int recv, int uid)
{
  Ptr<Node> node = olsrContainer.Get (recv);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ipv4Address recvaddr = ipv4->GetAddress (1, 0).GetLocal ();

  Ipv4Address table_addr[10];
  olsrh.aaa (NodeList::GetNode (sender), table_addr);

  for (int i = 0; i < 10; i++)
    {
      if (recvaddr == table_addr[i] && mem[uid * 10 + sender] == false)
        {
          int mode = 0;
          epidemic.Drop (dtnContainer, uid);
          olsr_testApp (recv, sender, mode, uid);
          break;
        }
    }
}

void
olsr_testApp (int recv, int sender, int mode, int uid)
{
  int numPackets = 1;
  // Create UDP application at n0
  Address sinkAddress1 (InetSocketAddress (interfaces.GetAddress (recv),
                                           sinkPort)); // interface of n24
  Ptr<Socket> ns3UdpSocket1;
  ns3UdpSocket1 = Socket::CreateSocket (olsrContainer.Get (sender),
                                        UdpSocketFactory::GetTypeId ()); //
  Ptr<MyApp> app1 = CreateObject<MyApp> ();

  app1->Setup (ns3UdpSocket1, sinkAddress1, sender, recv, packetSize, numPackets, mode, uid,
               DataRate ("1Mbps"));
  app1->SetStartTime (Seconds (0.));
  app1->SetStopTime (Seconds (1));

  olsrContainer.Get (sender)->AddApplication (app1);
}

void
dtn_testApp (int recv, int sender, int mode, int uid)
{
  int numPackets = 1;
  // Create UDP application at n0
  Address sinkAddress2 (InetSocketAddress (interfaces2.GetAddress (recv),
                                           sinkPort)); // interface of n24
  Ptr<Socket> ns3UdpSocket2;
  ns3UdpSocket2 = Socket::CreateSocket (dtnContainer.Get (sender),
                                        UdpSocketFactory::GetTypeId ()); //
  Ptr<MyApp> app2 = CreateObject<MyApp> ();

  app2->Setup (ns3UdpSocket2, sinkAddress2, sender, recv, packetSize, numPackets, mode, uid,
               DataRate ("1Mbps"));
  app2->SetStartTime (Seconds (0.));
  app2->SetStopTime (Seconds (1));

  dtnContainer.Get (sender)->AddApplication (app2);
}

/**
 *  Test OLSR 
 */

void
makeconvarray ()
{
  for (int i = 0; i < numberOfueNodes; i++)
    {
      Ptr<Node> olsr_node = olsrContainer.Get (i);
      Ptr<Ipv4> olsr_ipv4 = olsr_node->GetObject<Ipv4> ();

      Ptr<Node> dtn_node = dtnContainer.Get (i);
      Ptr<Ipv4> dtn_ipv4 = dtn_node->GetObject<Ipv4> ();
      //NS_LOG_INFO( "(makeconvarray) convarray[" << olsr_ipv4->GetAddress (1, 0).GetLocal () << "] = " << Ipv4Address (dtn_ipv4->GetAddress (1, 0).GetLocal ()));
      convarray[olsr_ipv4->GetAddress (1, 0).GetLocal ()] =
          Ipv4Address (dtn_ipv4->GetAddress (1, 0).GetLocal ());
      convarray_node_dtnaddr[i] =  Ipv4Address (dtn_ipv4->GetAddress (1, 0).GetLocal ());
      convarray_dtnaddr_node[Ipv4Address (dtn_ipv4->GetAddress (1, 0).GetLocal ())] = i;
    }
}

void
makeconvarray_node_olsraddr ()
{
  for (int i = 0; i < numberOfueNodes; i++)
    {
      Ptr<Node> olsr_node = olsrContainer.Get (i);
      Ptr<Ipv4> olsr_ipv4 = olsr_node->GetObject<Ipv4> ();

      convarray_node_olsraddr[i] = Ipv4Address (olsr_ipv4->GetAddress (1, 0).GetLocal ());
      convarray_olsraddr_node[Ipv4Address (olsr_ipv4->GetAddress (1, 0).GetLocal ())] = i;
    }
}

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Time::SetResolution (Time::NS);
  //LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  //LogComponentEnable("EpidemicRoutingProtocol", LOG_LEVEL_WARN);
  LogComponentEnable ("test-olsropp", LOG_LEVEL_INFO);

  olsrContainer.Create (numberOfueNodes);
  dtnContainer.Create (numberOfueNodes);

  /*
   *      Mobility model Setup
   */

  MobilityHelper mobility;

  Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator> ();
  initialAlloc->Add (Vector (-100, 0, 1.5));
  initialAlloc->Add (Vector (250, 0, 1.5));
  initialAlloc->Add (Vector (500, 0, 1.5));
  initialAlloc->Add (Vector (750, 0, 1.5));
  initialAlloc->Add (Vector (1000, 0, 1.5));
  mobility.SetPositionAllocator (initialAlloc);

  mobility.SetMobilityModel ("ns3::WaypointMobilityModel");
  mobility.Install (olsrContainer);
  mobility.Install (dtnContainer);

  Ptr<WaypointMobilityModel> waypoint =
    DynamicCast<WaypointMobilityModel> (dtnContainer.Get (0)->GetObject<MobilityModel> ());
  waypoint->AddWaypoint (Waypoint(Seconds(70), Vector(-100, 0, 1.5)));
  waypoint->AddWaypoint (Waypoint(Seconds(70.5), Vector(0, 0, 1.5)));
  waypoint->AddWaypoint (Waypoint(Seconds(89.5), Vector(0, 0, 1.5)));
  waypoint->AddWaypoint (Waypoint(Seconds(90), Vector(-100, 0, 1.5)));

  Ptr<WaypointMobilityModel> o_waypoint =
    DynamicCast<WaypointMobilityModel> (olsrContainer.Get (0)->GetObject<MobilityModel> ());
  o_waypoint->AddWaypoint (Waypoint(Seconds(70), Vector(-100, 0, 1.5)));
  o_waypoint->AddWaypoint (Waypoint(Seconds(70.5), Vector(0, 0, 1.5)));
  o_waypoint->AddWaypoint (Waypoint(Seconds(89.5), Vector(0, 0, 1.5)));
  o_waypoint->AddWaypoint (Waypoint(Seconds(90), Vector(-100, 0, 1.5)));

  Ptr<WaypointMobilityModel> waypoint2 =
    DynamicCast<WaypointMobilityModel> (dtnContainer.Get (4)->GetObject<MobilityModel> ());
  waypoint2->AddWaypoint (Waypoint(Seconds(60), Vector(1000, 0, 1.5)));
  waypoint2->AddWaypoint (Waypoint(Seconds(60.5), Vector(1050, 0, 1.5)));
  waypoint2->AddWaypoint (Waypoint(Seconds(95.5), Vector(1050, 0, 1.5)));
  waypoint2->AddWaypoint (Waypoint(Seconds(100), Vector(1000, 0, 1.5)));

  Ptr<WaypointMobilityModel> o_waypoint2 =
    DynamicCast<WaypointMobilityModel> (olsrContainer.Get (4)->GetObject<MobilityModel> ());
  o_waypoint2->AddWaypoint (Waypoint(Seconds(60), Vector(1000, 0, 1.5)));
  o_waypoint2->AddWaypoint (Waypoint(Seconds(60.5), Vector(1050, 0, 1.5)));
  o_waypoint2->AddWaypoint (Waypoint(Seconds(95.5), Vector(1050, 0, 1.5)));
  o_waypoint2->AddWaypoint (Waypoint(Seconds(100), Vector(1000, 0, 1.5)));


  /*
   *       Physical and link Layer Setup
   */

  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("Frequency", UintegerValue (2400)); //2400
  wifiPhy.Set ("TxPowerEnd", DoubleValue (30));
  wifiPhy.Set ("TxPowerStart", DoubleValue (30));
  wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue (-80));
  //wifiPhy.Set ("CcaModelThreshold", DoubleValue(-90));

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);

  devices = wifi.Install (wifiPhy, wifiMac, olsrContainer);
  devices2 = wifi.Install (wifiPhy, wifiMac, dtnContainer);

  /*
  *    Routing Setup
  */

  uint32_t epidemicHopCount = 1000;
  uint32_t epidemicQueueLength = 1000;
  Time epidemicQueueEntryExpireTime = Seconds (700);
  Time epidemicBeaconInterval = Seconds (1);

  epidemic.Set ("HopCount", UintegerValue (epidemicHopCount));
  epidemic.Set ("QueueLength", UintegerValue (epidemicQueueLength));
  epidemic.Set ("QueueEntryExpireTime", TimeValue (epidemicQueueEntryExpireTime));
  epidemic.Set ("BeaconInterval", TimeValue (epidemicBeaconInterval));

  Ipv4StaticRoutingHelper staticRouting;
  list.Add (staticRouting, 0);
  list.Add (olsrh, 10);

  /*
 *     Internet Stack Setup
 */

  InternetStackHelper internet;
  internet.SetRoutingHelper (list);
  internet.Install (olsrContainer);
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  interfaces = ipv4.Assign (devices);

  InternetStackHelper internet2;
  internet2.SetRoutingHelper (epidemic);
  internet2.Install (dtnContainer);
  Ipv4AddressHelper ipv4_2;
  ipv4_2.SetBase ("192.168.2.0", "255.255.255.0");
  interfaces2 = ipv4_2.Assign (devices2);

  /*
   *     Application Setup     
   */
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory",
                                     InetSocketAddress (Ipv4Address::GetAny (), sinkPort));

  ApplicationContainer sinkApps2;
  sinkApps2 = packetSinkHelper.Install (olsrContainer); //n2 as sink
  sinkApps2 = packetSinkHelper.Install (dtnContainer); //n2 as sink
  sinkApps2.Start (Seconds (0.));
  sinkApps2.Stop (Seconds (1000.));

  int mode = 0;
  int uid = -1;
  Time timer = Seconds (50);

  makeconvarray ();
  makeconvarray_node_olsraddr ();

  Simulator::Schedule (timer, &olsr_testApp, RECV, SENDER, mode, uid);

  /*
  *   others
  */
  AsciiTraceHelper ascii;
  wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("test-tracesource.tr"));
  wifiPhy.EnablePcapAll ("test-proposal-0710");

  Simulator::Stop (Seconds (appTotalTime));

  AnimationInterface anim ("test-olsropp-animation.xml");
  anim.EnablePacketMetadata (true);
  anim.EnableIpv4RouteTracking ("tinkoko.xml", Seconds (0), Seconds (appDataEnd), Seconds (0.25));

  /*
   *  tracing olsr routingtable
   */
  //list.PrintRoutingTableAt(timer, olsrContainer.Get(1), streamer);

  // Mandatory
  Config::Connect ("NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRxDrop", MakeCallback (&MacRxDrop));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTxDrop", MakeCallback (&MacTxDrop));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback (&PhyTxDrop));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback (&PhyRxDrop));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxBegin", MakeCallback (&PhyTxBegin));
  //Config::Connect ("NodeList/*/$ns3::Ipv4L3Protocol/Drop", MakeCallback(&Drop));
  Config::Connect ("NodeList/*/$ns3::Ipv4L3Protocol/Rx", MakeCallback (&Rx));
  //Config::Connect ("NodeList/*/$ns3::Ipv4L3Protocol/Tx", MakeCallback (&Tx));
  //Config::Connect ("NodeList/*/$ns3::Ipv4L3Protocol/UnicastForward", MakeCallback (&UnicastForward));
  //Config::Connect ("NodeList/*/$ns3::Ipv4L3Protocol/SendOutgoing", MakeCallback (&SendOutgoing));
  //Config::Connect ("NodeList/*/$ns3::Ipv4L3Protocol/LocalDeliver", MakeCallback (&LocalDeliver));
  Config::Connect ("NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback (&RxPS));
  //Config::Connect ("NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback (&ReceivedPacket));
  //Config::Connect("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/State/RxError", MakeCallback(&RxError));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx",MakeCallback (&MonitorSnifferRx));
  //Config::Connect ("NodeList/*/$ns3::Epidemic::RoutingProtocol/TraceSendPacket", MakeCallback (&TraceSendPacket));
  Config::Connect ("NodeList/*/$ns3::Epidemic::RoutingProtocol/TraceupdateDeliveryPredFor",
                   MakeCallback (&TraceupdateDeliveryPredFor));
  Config::Connect ("NodeList/*/$ns3::Epidemic::RoutingProtocol/TraceupdateTransitivePreds",
                   MakeCallback (&TraceupdateTransitivePreds));
  Config::Connect ("NodeList/*/$ns3::Epidemic::RoutingProtocol/TracesendPacketFromQueue", 
                    MakeCallback (&TraceSendPacketFromQueue));
  Config::Connect ("NodeList/*/$ns3::Epidemic::RoutingProtocol/TraceRouteInput",
                   MakeCallback (&TraceRouteInput));
  Config::Connect ("NodeList/*/$ns3::Epidemic::RoutingProtocol/TraceGetJudge_Queuelist",
                   MakeCallback (&GetJudge_Queuelist));
  Config::Connect ("NodeList/*/$ns3::Epidemic::RoutingProtocol/Tracepastcontact", MakeCallback (&Tracepastcontact));
  Config::Connect ("NodeList/*/$ns3::Epidemic::RoutingProtocol/TraceJudgeFrompastcontact", MakeCallback (&TraceJudgeFrompastcontact));
  Config::Connect ("NodeList/*/$ns3::olsr::RoutingProtocol/TraceAddEntry",
                   MakeCallback (&TraceAddEntry));
  Config::Connect ("NodeList/*/$ns3::ArpL3Protocol/CacheList/*/Drop",
                   MakeCallback (&ArpDrop));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}