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
uint32_t packetSize = 1000; // 2048
double appTotalTime = 1000; // simulator stop time
double appDataStart = 0.0;
double appDataEnd = 1000;
int numberOfueNodes = 9;
int numPackets = 1;
uint16_t sinkPort = 6; // use the same for all apps
int SENDER = 0;
double nodeSpeed = 2.17;

// From the UID and the sender in tag, check whether the source is sending packet with a specific UID or not.
//std::map<int, bool> mem;
std::map<int, std::map<int, bool>> mem;
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

std::map <int, bool> send_count;
std::map <int, bool> recv_count;

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
void dtn_to_olsr (int sender, int recv, int inter, int uid);
/// Send a packet using the olsr application.
void olsr_testApp (int recv, int sender, int inter, int mode, int uid, bool promode);
/// Send a packet using the DTN application.
void dtn_testApp (int recv, int sender, int inter, int mode, int uid);

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
std::map<Ipv4Address, double> ExtractIpPred (Ptr<Node> node, int recv);
/// 提案手法の実行を行う関数
void do_proposal (int sender, int recv, int inter, int uid, int mode);

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

  void Setup (Ptr<Socket> socket, Address address, uint32_t senderAdress, uint32_t receiverAddress, uint32_t interAddress, 
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
  uint32_t m_inter;
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
      m_inter(1),
      m_mode (0),
      m_uid (-1)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t sender, uint32_t receiver, uint32_t inter, 
              uint32_t packetSize, uint32_t nPackets, uint32_t mode, int uid, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_sender = sender;
  m_receiver = receiver;
  m_inter = inter;
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
  if (mem[m_uid * 10 + m_sender][m_receiver] == false && !(m_sender == m_receiver)) 
      {
      IPv4TestTag tag;
      Ptr<Packet> packet = Create<Packet> (m_packetSize);

      if (m_uid < 0) // This packet is generated by sender
        {
          NS_LOG_INFO("(MyApp::SendPacket) Time: " << Simulator::Now().GetSeconds() << " sender " << m_sender << " send packet " << packet->GetUid() << " to receiver " << m_receiver <<  " mode:" << m_mode << " is started");
          send_count[packet->GetUid()] = true;
          tag.SetToken (packet->GetUid () * 100000 + m_receiver * 1000 + m_inter * 10 + m_mode);
          packet->AddPacketTag (tag);
          m_socket->Send (packet);
          mem[m_uid * 10 + m_sender][m_receiver] = true;
        }
      else // This packet is generated by intermediary node
        {
          tag.SetToken (m_uid * 100000 + m_receiver * 1000 + m_inter * 10 + m_mode);
          packet->AddPacketTag (tag);
          m_socket->Send (packet);
          mem[m_uid * 10 + m_sender][m_receiver] = true;
        }
      }

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
  NS_LOG_INFO(Simulator::Now () << ", kym=" << kym << ", pos=" << position << ", x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z);
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

  IPv4TestTag tagCopy;
  packet->PeekPacketTag (tagCopy);
  int token = tagCopy.GetToken ();

  int uid = token / 100000;
  int temp = token - uid * 100000;
  int recv = temp / 1000;
  int temp2 = temp - recv * 1000;
  int inter = temp2 / 10;
  int mode = temp2 - inter * 10;

  mem[uid*10+kym][recv] = false;  
  
  epidemic.setOpp_preds2 (dtnContainer, kym, mainmap[dst]);

  do_proposal(kym, recv, inter, uid, mode);
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

  int uid = token / 100000;
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

  int uid = token / 100000;
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
ExtractIpPred (Ptr<Node> node, int recv)
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
          ipandpred[route.destAddr] = route.preds[convarray_node_dtnaddr[recv]];
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
                  ipandpred[route2.destAddr] = route2.preds[convarray_node_dtnaddr[recv]];
                  //std::cout << "(ExtractIpPred) Time: " << Simulator::Now().GetSeconds()<< ", ipandpred[" << route2.destAddr << "] = route2.pred = " << ipandpred[route2.destAddr] << std::endl;
                  //NS_LOG_INFO("(ExtractIpPred) Time: " << Simulator::Now().GetSeconds()<< ", ipandpred[" << route2.destAddr << " = route2.pred = " << ipandpred[route2.destAddr]);
                }
            }
        }
    }
    return ipandpred;
}

void
do_proposal (int sender, int recv, int inter, int uid, int mode)
{
  //NS_LOG_INFO("(do_proposal) Time: " << Simulator::Now().GetSeconds() << " do_proposal()is started");

  mode = 1;
  std::map<Ipv4Address, double> ipandpred;

  // ノードが宛先ではない時
  if (sender != recv)
    {
      // 経路表のIPアドレスと宛先到達確率を配列として抜き出す
      ipandpred = ExtractIpPred (NodeList::GetNode (sender), recv);

      // 自分の宛先到達確率も抜き出す
      double s_pred = mainmap[convarray[convarray_node_olsraddr[sender]]][convarray[convarray_node_olsraddr[recv]]];

      // 経路表の宛先到達確率 > 自分の宛先到達確率となる、IPアドレスを持つノードを抽出する
      for (auto i = ipandpred.begin (); i != ipandpred.end (); ++i)
        {
          if (s_pred < i->second)
            {
              // 宛先到達確率が高いノードにOLSRを用いてパケットを送信する。この時、modeは1のまま送信する。
              olsr_testApp (recv, sender, convarray_olsraddr_node[i->first], mode, uid, true);
            }
        }
      // 特定のuidを蓄積しているノードが存在しない時、そのノードに特定のuidを持つパケットを蓄積するためにDTNでパケットを送信する
      if (queuelist[sender][uid] == false)
        {
          // DTNでパケットを送信する
          dtn_testApp (recv, sender, inter, mode, uid);
        }
    }
}

/*
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

  std::cout << "(ArpDrop):" << token << std::endl;

  int uid = token / 100000;
  int temp = token - uid * 100000;
  int recv = temp / 1000;
  int temp2 = temp - recv * 1000;
  int inter = temp2 / 10;
  int mode = temp2 - inter * 10;

  std::cout << "(ArpDrop):" << uid << ", " << recv << ", " << inter << ", " << mode << std::endl;

  mem[uid*10+kym][recv] = false;

  if (mode == 0 && kym < numberOfueNodes &&
      packet->GetSize () >= 2048) // OLSRモードでパケット損失した時DTNモードに変更する
    {
      //NS_LOG_INFO("(ArpCache/Drop) is started at " << kym << " due to drop (" << packet->GetUid () << ", " << packet->GetSize () << ")");
      do_proposal (kym, recv, inter, uid, mode);
    }
}
*/

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

  int uid = token / 100000;
  int temp = token - uid * 100000;
  int recv = temp / 1000;
  int temp2 = temp - recv * 1000;
  int inter = temp2 / 10;
  int mode = temp2 - inter * 10;

  std::map<Ipv4Address, double> ipandpred;

  if (mode == 1)
    {
      if (kym >= numberOfueNodes)
        {
          kym = kym - numberOfueNodes;
        }

      do_proposal (kym, recv, inter, uid, mode);
    }
    
    if (kym == recv)
    {
      NS_LOG_INFO ("(PacketSink/Rx) "
                       << "Time:" << Simulator::Now ().GetSeconds () << ", "
                       << "Success At "
                       << "packet->GetUid(): " << packet->GetUid () << ", "
                       << "uid:" << uid << ", "
                       << "recv:" << kym << ", "
                       << "mode:" << mode);
      std::cout << "(PacketSink/Rx) "
                       << "Time:" << Simulator::Now ().GetSeconds () << ", "
                       << "Success At "
                       << "packet->GetUid(): " << packet->GetUid () << ", "
                       << "uid:" << uid << ", "
                       << "recv:" << kym << ", "
                       << "mode:" << mode << std::endl;
      recv_count[uid] = true;
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
      
      //std::cout << "(Rx):" << token << std::endl;

      int uid = token / 100000;
      int temp = token - uid * 100000;
      int recv = temp / 1000;
      int temp2 = temp - recv * 1000;
      int inter = temp2 / 10;
      int mode = temp2 - inter * 10;

       //std::cout << "Time:" << Simulator::Now().GetSeconds() << ", (Rx):" << kym << ", " << uid << ", " << recv << ", " << inter << ", " << mode << std::endl;
      
      if (mode == 1)
      {
        if (kym >= numberOfueNodes){
          kym = kym - numberOfueNodes;
          if (kym < recv) // if node is relay node try to connect to OLSR
            {
              int time = Simulator::Now ().GetSeconds ();
              int endtime = (int) appDataEnd - time;
              for (int i = 0; i < endtime; i++) 
                {
                  Simulator::Schedule (Seconds (i), &dtn_to_olsr, kym, recv, inter, uid);

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
olsr_or_dtn (int sender, int recv, int uid)
{
  int mode = 0;
  int inter = 1;
  Ipv4Address table_addr[numberOfueNodes];
  olsrh.aaa (NodeList::GetNode (sender), table_addr);

  Ptr<Node> node = olsrContainer.Get (recv);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ipv4Address recvaddr = ipv4->GetAddress (1, 0).GetLocal ();

  for (int i = 0; i < numberOfueNodes; i++)
    {
      // 宛先アドレスとOLSRの経路上のDESTアドレスが一致した時にOLSRモードを使ってパケットを宛先に対して送信する。
      if (recvaddr == table_addr[i])
        {
          mode = 0;
          //std::cout << "do olsr mode " << std::endl;
          olsr_testApp (recv, sender, inter, mode, uid, false);
          break;
        }
      // OLSRの経路上のDESTアドレスが”102.102.102.102”になった時、宛先アドレスがOLSRの経路上に存在しないと判断し、DTNを使用する。
      if (table_addr[i] == Ipv4Address ("102.102.102.102"))
        {
          //std::cout << "Do DTN mode " << std::endl;
          mode = 1;
          std::map<Ipv4Address, double> ipandpred;
          // ノードが宛先ではない時
          if (sender != recv)
          {
              //std::cout << "1" << std::endl;
              // 経路表のIPアドレスと宛先到達確率を配列として抜き出す
              ipandpred = ExtractIpPred(NodeList::GetNode(sender), recv);
              // 自分の宛先到達確率も抜き出す
              double s_pred = mainmap[convarray[convarray_node_olsraddr[sender]]][convarray[convarray_node_olsraddr[recv]]];
              // 経路表の宛先到達確率 > 自分の宛先到達確率となる、IPアドレスを持つノードを抽出する
              for (auto i = ipandpred.begin(); i != ipandpred.end(); ++i)
              {
                  //std::cout << "2" << std::endl;
                  if (s_pred < i->second)
                  {
                      //std::cout << "Do olsr " << std::endl;
                      // 宛先到達確率が高いノードにOLSRを用いてパケットを送信する。この時、modeは1のまま送信する。
                      olsr_testApp(recv, sender, convarray_olsraddr_node[i->first], mode, uid, true);
                  }
              }
              // 
              if(queuelist[sender][uid] == false)
              {
                //std::cout << "Do DTN " << std::endl;
                // DTNでパケットを送信する
                dtn_testApp(recv, sender, inter, mode, uid);
              }
          }
          break;
        }
    }
}

void
dtn_to_olsr (int sender, int recv, int inter, int uid)
{
  Ptr<Node> node = olsrContainer.Get (recv);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ipv4Address recvaddr = ipv4->GetAddress (1, 0).GetLocal ();

  Ipv4Address table_addr[10];
  olsrh.aaa (NodeList::GetNode (sender), table_addr);

  for (int i = 0; i < 10; i++)
    {
      if (recvaddr == table_addr[i] && mem[uid * 10 + sender][recv] == false)
        {
          int mode = 0;
          epidemic.Drop (dtnContainer, uid);
          olsr_testApp (recv, sender, inter, mode, uid, false);
          break;
        }
    }
}

void
olsr_testApp (int recv, int sender, int inter, int mode, int uid, bool promode)
{
  int numPackets = 1;
  if (promode == false) 
  {
    Address sinkAddress1 (InetSocketAddress (interfaces.GetAddress (recv),
                                           sinkPort)); 

    Ptr<Socket> ns3UdpSocket1;
    ns3UdpSocket1 = Socket::CreateSocket (olsrContainer.Get (sender),
                                          UdpSocketFactory::GetTypeId ()); //
    Ptr<MyApp> app1 = CreateObject<MyApp> ();
    app1->Setup (ns3UdpSocket1, sinkAddress1, sender, recv, inter, packetSize, numPackets, mode, uid, DataRate ("1Mbps"));
    app1->SetStartTime (Seconds (0.));
    app1->SetStopTime (Seconds (1));

    olsrContainer.Get (sender)->AddApplication (app1);
  } 
  else 
  {
    Address sinkAddress1 (InetSocketAddress (interfaces.GetAddress (inter),
                                           sinkPort)); 
    Ptr<Socket> ns3UdpSocket1;
    ns3UdpSocket1 = Socket::CreateSocket (olsrContainer.Get (sender),
                                          UdpSocketFactory::GetTypeId ()); //
    Ptr<MyApp> app1 = CreateObject<MyApp> ();
    app1->Setup (ns3UdpSocket1, sinkAddress1, sender, recv, inter, packetSize, numPackets, mode, uid, DataRate ("1Mbps"));
    app1->SetStartTime (Seconds (0.));
    app1->SetStopTime (Seconds (1));

    olsrContainer.Get (sender)->AddApplication (app1);
  }
}

void
dtn_testApp (int recv, int sender, int inter, int mode, int uid)
{
  int numPackets = 1;
  // Create UDP application at n0
  Address sinkAddress2 (InetSocketAddress (interfaces2.GetAddress (recv),
                                           sinkPort)); // interface of n24
  Ptr<Socket> ns3UdpSocket2;
  ns3UdpSocket2 = Socket::CreateSocket (dtnContainer.Get (sender),
                                        UdpSocketFactory::GetTypeId ()); //
  Ptr<MyApp> app2 = CreateObject<MyApp> ();

  app2->Setup (ns3UdpSocket2, sinkAddress2, sender, recv, inter, packetSize, numPackets, mode, uid,
               DataRate ("1Mbps"));
  app2->SetStartTime (Seconds (0.));
  app2->SetStopTime (Seconds (1));

  dtnContainer.Get (sender)->AddApplication (app2);
}

int intv_uid = 0;

void
sendpacket_interval ()
{
  intv_uid--;
  for (int i = 6; i < numberOfueNodes; i++)
  {
    for (int j = 6; j < numberOfueNodes; j++)
    {
      if (i != j)
        olsr_or_dtn (i, j, intv_uid);
    }
    Ptr<MobilityModel> mobility = olsrContainer.Get(i)->GetObject<MobilityModel>();
    Vector current = mobility->GetPosition();
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ", node " << i << "'s position is " << current);
  }
  NS_LOG_INFO(Simulator::Now().GetSeconds() << ", send_count, " << send_count.size() << ", recv_count, " << recv_count.size());
}

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

void 
DropbyBufferoverflow (std::string context, Ptr<const Packet> packet) 
{
    NS_LOG_INFO ("DROP DUE TO BUFFER OVERFLOW");
}

void
DropByNotenoughroom (std::string context, Ptr<const Packet> packet)
{
    NS_LOG_INFO (Simulator::Now().GetSeconds() << ", " <<  packet->GetUid() << ", DROP DUE TO NOT ENOUGH ROOM");
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
  //LogComponentEnable("EpidemicPacketQueue", LOG_LEVEL_WARN);
  LogComponentEnable ("test-olsropp", LOG_LEVEL_INFO);

  olsrContainer.Create (numberOfueNodes);
  dtnContainer.Create (numberOfueNodes);

  /*
   *      Mobility model Setup
   */

  MobilityHelper mobility;
  MobilityHelper mobility1;
  MobilityHelper mobility2;
  MobilityHelper mobility3;
  //MobilityHelper mobility4;
  //MobilityHelper mobility5;
  //MobilityHelper mobility6;

  Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator> ();
  initialAlloc->Add (Vector (500, 500, 200));
  initialAlloc->Add (Vector (1500, 500, 200));
  initialAlloc->Add (Vector (2500, 500, 200));
  initialAlloc->Add (Vector (500, 1500, 200));
  initialAlloc->Add (Vector (1500, 1500, 200));
  initialAlloc->Add (Vector (2500, 1500, 200));
  //initialAlloc->Add (Vector (1823, 1820, 1.5));
  //initialAlloc->Add (Vector (223, 1703, 1.5));
  //initialAlloc->Add (Vector (2452, 1540, 1.5));
  initialAlloc->Add (Vector (1900, 900, 1.5));
  initialAlloc->Add (Vector (900, 1900, 1.5));
  initialAlloc->Add (Vector (2900, 1900, 1.5));

  mobility.SetPositionAllocator (initialAlloc);
  mobility1.SetPositionAllocator (initialAlloc);
  mobility2.SetPositionAllocator (initialAlloc);
  mobility3.SetPositionAllocator (initialAlloc);
  //mobility4.SetPositionAllocator (initialAlloc);
  //mobility5.SetPositionAllocator (initialAlloc);
  //mobility6.SetPositionAllocator (initialAlloc);
  

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install (olsrContainer);
  mobility.Install (dtnContainer);

  for (int i = 0; i <= 5; i++)
  {
    mobility.Install (olsrContainer.Get(i));
    mobility.Install (dtnContainer.Get(i));
  }

  mobility1.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", 
                              "Bounds", StringValue("1000|2000|0|1000"),
                              "Speed", StringValue("ns3::UniformRandomVariable[Min=10|Max=10]"));
  
  mobility1.Install(olsrContainer.Get(6));
  mobility1.Install(dtnContainer.Get(6));


  mobility2.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", 
                              "Bounds", StringValue("0|1000|1000|2000"),
                              "Speed", StringValue("ns3::UniformRandomVariable[Min=2.17|Max=2.17]"));
  
  mobility2.Install(olsrContainer.Get(7));
  mobility2.Install(dtnContainer.Get(7));

  mobility3.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", 
                              "Bounds", StringValue("2000|3000|1000|2000"),
                              "Speed", StringValue("ns3::UniformRandomVariable[Min=10|Max=10]"));
  
  mobility3.Install(olsrContainer.Get(8));
  mobility3.Install(dtnContainer.Get(8));

  /*
  mobility1.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX", DoubleValue (0), "MaxX", DoubleValue (1000), "MinPause", DoubleValue (0), "MaxPause", DoubleValue (0), "MinY", DoubleValue (0), "MaxY", DoubleValue (1000), "Z", DoubleValue (1.5));
  
  mobility1.Install(olsrContainer.Get(6));
  mobility1.Install(dtnContainer.Get(6));

  mobility2.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX", DoubleValue (1000), "MaxX", DoubleValue (2000), "MinPause", DoubleValue (0), "MaxPause", DoubleValue (0), "MinY", DoubleValue (0), "MaxY", DoubleValue (1000), "Z", DoubleValue (1.5));
  
  mobility2.Install(olsrContainer.Get(7));
  mobility2.Install(dtnContainer.Get(7));

  mobility3.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX", DoubleValue (2000), "MaxX", DoubleValue (3000), "MinPause", DoubleValue (0), "MaxPause", DoubleValue (0), "MinY", DoubleValue (0), "MaxY", DoubleValue (1000), "Z", DoubleValue (1.5));
  
  mobility3.Install(olsrContainer.Get(8));
  mobility3.Install(dtnContainer.Get(8));

  mobility4.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX", DoubleValue (0), "MaxX", DoubleValue (1000), "MinPause", DoubleValue (0), "MaxPause", DoubleValue (0), "MinY", DoubleValue (1000), "MaxY", DoubleValue (2000), "Z", DoubleValue (1.5));
  
  mobility4.Install(olsrContainer.Get(9));
  mobility4.Install(dtnContainer.Get(9));

  mobility5.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX", DoubleValue (1000), "MaxX", DoubleValue (2000), "MinPause", DoubleValue (0), "MaxPause", DoubleValue (0), "MinY", DoubleValue (1000), "MaxY", DoubleValue (2000), "Z", DoubleValue (1.5));
  
  mobility5.Install(olsrContainer.Get(10));
  mobility5.Install(dtnContainer.Get(10));

  mobility6.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX", DoubleValue (2000), "MaxX", DoubleValue (3000), "MinPause", DoubleValue (0), "MaxPause", DoubleValue (0), "MinY", DoubleValue (1000), "MaxY", DoubleValue (2000), "Z", DoubleValue (1.5));
  
  mobility6.Install(olsrContainer.Get(11));
  mobility6.Install(dtnContainer.Get(11));
  */
  /*
   *       Physical and link Layer Setup
   */

  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("Frequency", UintegerValue (2400)); //2400
  wifiPhy.Set ("TxPowerEnd", DoubleValue (21));
  wifiPhy.Set ("TxPowerStart", DoubleValue (21));
  wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue (-87));
  //wifiPhy.Set ("CcaModelThreshold", DoubleValue(-90));

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel");
  //wifiChannel.AddPropagationLoss ("ns3::ModifiedHataModel");
  //wifiChannel.AddPropagationLoss("ns3::ExtendedHataModel");
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
  sinkApps2.Stop (Seconds (appDataEnd));

  makeconvarray ();
  makeconvarray_node_olsraddr ();

  for (int i = 30; i < (int) appTotalTime; i = i + 30)
  {
    Simulator::Schedule (Seconds(i), &sendpacket_interval);
  }

  /*
  *   others
  */
  AsciiTraceHelper ascii;
  wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("test-tracesource.tr"));
  wifiPhy.EnablePcapAll ("test-proposal-0807");

  Simulator::Stop (Seconds (appTotalTime));

  //AnimationInterface anim ("test-olsropp-animation.xml");
  //anim.EnablePacketMetadata (true);
  //anim.EnableIpv4RouteTracking ("tinkoko.xml", Seconds (0), Seconds (appDataEnd), Seconds (0.25));

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
  //Config::Connect ("NodeList/*/$ns3::ArpL3Protocol/CacheList/*/Drop",
  //                 MakeCallback (&ArpDrop));
  Config::Connect("NodeList/*/$ns3::UdpL4Protocol/SocketList/*/Drop", MakeCallback(&DropbyBufferoverflow));
  Config::Connect("NodeList/*/$ns3::ArpL3Protocol/Drop", MakeCallback(&DropByNotenoughroom));
  

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}