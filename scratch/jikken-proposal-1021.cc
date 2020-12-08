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
//std::string phyMode ("DsssRate1Mbps");
//std::string phyMode ("OfdmRate12Mbps");
std::string phyMode ("OfdmRate54Mbps");
std::string ControlMode ("OfdmRate6Mbps");
std::string rate = "200Mbps"; // 1.024kbps
uint32_t packetSize = 1000; // 2048
double appDataStart = 0.0;
double appDataEnd = 3560;
double appTotalTime = appDataStart + appDataEnd; // simulator stop time
int numberOfueNodes = 9;
int numberOfvehicles = 6;
int pre_numberOfvehicles = 3;
int numPackets = 1;
uint16_t sinkPort = 6; // use the same for all apps

// From the UID and the sender in tag, check whether the source is sending packet with a specific UID or not.
//std::map<int, bool> mem;
std::map<int, std::map<int, bool>> mem;

// 送信元がUID > 0のパケットを送信したときに、それらのパケットを保存するための配列
std::map<int, std::map<int, bool>> start_mem;

// An array that stores the destination arrival probability of prophet (mainaddress, neighboraddress, prediction)
std::map<Ipv4Address, std::map<Ipv4Address, double>> mainmap;

// Array for converting OLSR address to DTN address (OLSR address, DTN address)
std::map<Ipv4Address, Ipv4Address> convarray;

// ノード番号からOLSRのアドレスへの変換を行う配列
std::map<int, Ipv4Address> convarray_node_olsraddr;

// OLSRのアドレスからノード番号への変換を行う配列
std::map<Ipv4Address, int> convarray_olsraddr_node;

// ノード番号からDTNのアドレスへと変換する配列
std::map<int, Ipv4Address> convarray_node_dtnaddr;

// DTNのアドレスからノード番号へと変換する配列
std::map<Ipv4Address, int> convarray_dtnaddr_node;

// DTNに保存されるパケットのリスト
std::map<int, std::map<int, bool>> queuelist;

// 過去に接触したことのあるノードのリスト
std::map<Ipv4Address, std::map<Ipv4Address, bool>> main_pastcontact;

// 送信回数をカウントするための配列
std::map<int, bool> send_count;
// 受信回数をカウントするための配列
std::map<int, bool> recv_count;

// OLSRのルーティングテーブルの更新時間を記録する配列
std::map<int, double> tablechangelist;

/**
 *  prototype sengen
 */

// TRACE_SOURCE (FIRST:OLSROPP, SECOND:PROPHET, THIRD:PROPOSAL)

/// Ipv4L3Protocol/Rx : Switching from DTN to OLSR at relay node
void Rx (std::string context, Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t hate);

/// Ipv4L3Protocol/Drop : Swiching from OLSR to DTN when packet is dropped
void Drop (std::string context, const Ipv4Header &, Ptr<const Packet> packet,
           Ipv4L3Protocol::DropReason reason, Ptr<Ipv4> ipv4, uint32_t interface);

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
void dtn_to_olsr (Ptr<const Packet> packet, int sender, int recv, int inter, int uid);
/// Send a packet using the olsr application.
void olsr_testApp (Ptr<const Packet> packet, int recv, int sender, int inter, int mode, int uid,
                   bool promode);
/// Send a packet using the DTN application.
void dtn_testApp (Ptr<const Packet> packet, int recv, int sender, int inter, int mode, int uid);

// PROPOSAL

/// Substitute destination arrival probability of ProPHET in mainmap of main.cc
void test_olsrroutingtable ();
/// Display routing table of OLSR with destination arrival probability
void test_olsrroutingtable2 ();
/// Create a convarray that changes the OLSR IP address to the DTN IP address.
void makeconvarray ();
/// ノードがどのパケットを保存しているか管理する
void TraceRouteInput (std::string context, Ptr<const Packet> packet);
/// 冗長したパケットを保存しないようにする
void GetJudge_Queuelist (std::string context, Ptr<const Packet> packet);
/// ノードの宛先到達確率を抜き出す
std::map<Ipv4Address, double> ExtractIpPred (Ptr<Node> node, int recv);
/// 提案手法の実行を行う関数
void do_proposal (Ptr<const Packet> packet, int sender, int recv, int inter, int uid, int mode);

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

  void Setup (Ptr<const Packet> packet, Ptr<Socket> socket, Address address, uint32_t senderAdress,
              uint32_t receiverAddress, uint32_t interAddress, uint32_t packetSize,
              uint32_t nPackets, uint32_t mode, int uid, DataRate dataRate);
  void ChangeRate (DataRate newrate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<const Packet> m_packet;
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
    : m_packet (),
      m_socket (0),
      m_peer (),
      m_packetSize (0),
      m_nPackets (0),
      m_dataRate (0),
      m_sendEvent (),
      m_running (false),
      m_packetsSent (0),
      m_sender (0),
      m_receiver (0),
      m_inter (1),
      m_mode (0),
      m_uid (-1)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<const Packet> packet, Ptr<Socket> socket, Address address, uint32_t sender,
              uint32_t receiver, uint32_t inter, uint32_t packetSize, uint32_t nPackets,
              uint32_t mode, int uid, DataRate dataRate)
{
  m_packet = packet;
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
  //if (mem[m_uid * 10 + m_sender][m_receiver] == false && !(m_sender == m_receiver))
  if (!(m_sender == m_receiver))
    {
      IPv4TestTag tag;
      Ptr<Packet> new_packet = Create<Packet> (m_packetSize);

      if (m_uid < 0) // This packet is generated by sender
        {
          if (start_mem[m_sender][m_uid * 10 + m_receiver] == false)
            {
              NS_LOG_INFO ("(MyApp::SendPacket) Time:, " << Simulator::Now ().GetSeconds () 
                           << ", sender, " << m_sender
                           << ", send packet, " << new_packet->GetUid () 
                           << ", to inter, " << m_inter 
                           << ", to receiver, " << m_receiver 
                           << ", mode:, " << m_mode << ", is started");
              send_count[new_packet->GetUid ()] = true;
              tag.SetToken (new_packet->GetUid () * 100000 + m_receiver * 1000 + m_inter * 10 +
                            m_mode);
              new_packet->AddPacketTag (tag);
              m_socket->Send (new_packet);
              start_mem[m_sender][m_uid * 10 + m_receiver] = true;
            }
        }
      else // This packet is generated by intermediary node
        {
          if (mem[m_sender][m_uid * 10 + m_receiver] == false)
            {
              //NS_LOG_INFO("(MyApp::SendPacket), " << m_sender << ", " << m_uid << ", " << m_receiver);
              Ptr<Packet> old_packet = m_packet->Copy ();
              IPv4TestTag tagCopy;
              old_packet->PeekPacketTag (tagCopy);

              uint64_t token = tagCopy.GetToken ();

              std::cout << "(MyApp::SendPacket) Time:, " << Simulator::Now ().GetSeconds ()
                        << ", sender, " << m_sender
                        << ", m_packet->Copy() uid is, " << old_packet->GetUid ()
                        << ", to inter, " << m_inter
                        << ", to receiver, " << m_receiver
                        << ", mode:, " << m_mode 
                        << ", old_packet's tag token is, " << token << std::endl;

              m_socket->Send (m_packet->Copy ());
              //mem[m_uid * 10 + m_sender][m_receiver] = true;
            }
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
  NS_LOG_INFO ("(CourseChange), " << Simulator::Now ().GetSeconds() << ", kym=, " << kym << ", pos=, "
                                  << position << ", x=, " << pos.x << ", y=, " << pos.y << ", z=, "
                                  << pos.z);
  double distance;
  if (6 <= kym && kym <= 8)
  {
    if (kym == 6)
    {
      distance = sqrt(pow(pos.x - 500, 2) + pow(pos.y - 1500, 2));
    }
    else if (kym == 7)
    {
      distance = sqrt(pow(pos.x - 1500, 2) + pow(pos.y - 500, 2));
    }
    else if (kym == 8)
    {
      distance = sqrt(pow(pos.x - 2500, 2) + pow(pos.y - 1500, 2));
    }
    NS_LOG_INFO("(CourseChange2), " << Simulator::Now().GetSeconds() << ", kym=, " << kym << ", distance=, " << distance);
  }
}

/**
 *  PROPHETを実行するための関数群
 */

void
TraceupdateDeliveryPredFor (std::string context, Ipv4Address mainAddress, Ipv4Address addr,
                            double newValue)
{
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
  uint64_t token = tagCopy.GetToken ();

  uint64_t uid = token / 100000;
  uint64_t temp = token - uid * 100000;
  uint64_t recv = temp / 1000;
  uint64_t temp2 = temp - recv * 1000;
  uint64_t inter = temp2 / 10;
  uint64_t mode = temp2 - inter * 10;

  epidemic.setOpp_preds2 (dtnContainer, kym, mainmap[dst]);

  NS_LOG_INFO("(TraceSendPacketFromQueue), uid, " << uid << ", recv, " << recv << ", inter, " << inter << ", mode, " << mode);

  do_proposal(packet, kym, recv, inter, uid, mode);
}

void
GetJudge_Queuelist (std::string context, Ptr<const Packet> packet)
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

  uint64_t token = tagCopy.GetToken ();

  uint64_t uid = token / 100000;
  uint64_t temp = token - uid * 100000;
  uint64_t recv = temp / 1000;

  bool judge;

  if (queuelist[kym][uid * 10 + recv] == true)
    judge = true;
  else
    judge = false;

  Ptr<Node> node = dtnContainer.Get (kym);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
  Ptr<Epidemic::RoutingProtocol> epi = DynamicCast<Epidemic::RoutingProtocol> (proto);
  epi->GetBool (judge);

  //NS_LOG_INFO("(TraceGetJudge_Queuelist) kym is " << kym << ", uid is " << uid << ", packet size is " << packet->GetSize() << ", judge is " << judge );
}

void
Tracepastcontact (std::string context, Ipv4Address address, Ipv4Address address2, bool judge)
{
  main_pastcontact[address][address2] = judge;
  main_pastcontact[address2][address] = judge;
}

void
TraceJudgeFrompastcontact (std::string context, Ipv4Address address, Ipv4Address address2,
                           Ipv4Address address3)
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
      Ptr<Node> node = dtnContainer.Get (kym - numberOfueNodes);
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
      Ptr<Epidemic::RoutingProtocol> epi = DynamicCast<Epidemic::RoutingProtocol> (proto);
      epi->JudgeFrompastcontact (true);
    }
}

void
TraceRouteInput (std::string context, Ptr<const Packet> packet)
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

  uint64_t token = tagCopy.GetToken ();

  uint64_t uid = token / 100000;
  uint64_t temp = token - uid * 100000;
  uint64_t recv = temp / 1000;
  uint64_t temp2 = temp - recv * 1000;
  uint64_t inter = temp2 / 10;

  if (0 < token && token < 947000000000)
  {
    if (packet->GetSize() > packetSize)
    {
      NS_LOG_INFO("(TraceRouteInput), " << "time, " << Simulator::Now().GetSeconds() << ", kym, " << kym << ", token, " << token << ", uid, " << uid << ", recv, " << recv << ", inter, " << inter << ", packetsize, " << packet->GetSize());
      queuelist[kym][uid * 10 + recv] = true;
    }
  }
}

/**
 * 提案手法を実行するための準備をする関数
 */

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

  olsrh.setpred (olsrContainer, kym, mainmap[convarray[dest]]);
}

/**
 *  以下、提案手法を実行するための関数を記述
 */

// OLSRの経路表から宛先到達確率を抜き出したい時にこの関数が使われる
std::map<Ipv4Address, double>
ExtractIpPred (Ptr<Node> node, int recv)
{
  std::cout << "ExtractIpPred is started" << std::endl;
  std::map<Ipv4Address, double> ipandpred;

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
          // std::cout << "destAddr, " << route.destAddr << ", destpred, " << route.preds[convarray_node_dtnaddr[recv]] << ", ";
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
              // std::cout << "table2.size() is, " << table2.size() << ", ";
              for (uint32_t i = 0; i < table2.size (); ++i)
                {
                  olsr::RoutingTableEntry route2 = table2[i];
                  // std::cout <<  "dest, " << route2.destAddr << ", pred, " << route2.preds[convarray_node_dtnaddr[recv]] << ", ";
                  ipandpred[route2.destAddr] = route2.preds[convarray_node_dtnaddr[recv]];
                  //NS_LOG_INFO("(ExtractIpPred) Time: " << Simulator::Now().GetSeconds()<< ", ipandpred[" << route2.destAddr << " = route2.pred = " << ipandpred[route2.destAddr]);
                }
            }
        }
    }
  std::cout << std::endl;
  return ipandpred;
}

// 最初に送信元が提案手法でパケットを送信する時にこの関数が使われる
void
olsr_or_dtn (int sender, int recv, int uid)
{
  int mode = 0;
  int inter = 99;
  Ipv4Address table_addr[numberOfueNodes];
  olsrh.aaa (NodeList::GetNode (sender), table_addr);

  Ptr<Node> node = olsrContainer.Get (recv);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ipv4Address recvaddr = ipv4->GetAddress (1, 0).GetLocal ();

  Ptr<Packet> packet = Create<Packet> (packetSize);

  for (int i = 0; i < numberOfueNodes; i++)
    {
      // 宛先アドレスとOLSRの経路上のDESTアドレスが一致した時にOLSRモードを使ってパケットを宛先に対して送信する。
      if (recvaddr == table_addr[i])
        {
          mode = 0;
          olsr_testApp (packet, recv, sender, inter, mode, uid, false);
          mem[sender][uid*10+recv] = true;
          break;
        }
      // OLSRの経路上のDESTアドレスが”102.102.102.102”になった時、宛先アドレスがOLSRの経路上に存在しないと判断し、DTNを使用する。
      if (table_addr[i] == Ipv4Address ("102.102.102.102"))
        {
          mode = 1;
          std::map<Ipv4Address, double> ipandpred;
          // ノードが宛先ではない時
          if (sender != recv)
            {
              // 経路表のIPアドレスと宛先到達確率を配列として抜き出す
              ipandpred = ExtractIpPred (NodeList::GetNode (sender), recv);
              // 自分の宛先到達確率も抜き出す
              double s_pred = mainmap[convarray[convarray_node_olsraddr[sender]]]
                                     [convarray[convarray_node_olsraddr[recv]]];
              std::cout << "(do_proposal) time, " << Simulator::Now ().GetSeconds () << ", recv, "
                        << recv << ", sender, " << sender << ", pred is, " << s_pred << ", ";

              // 経路表の宛先到達確率 > 自分の宛先到達確率となる、IPアドレスを持つノードを抽出する
              for (auto i = ipandpred.begin (); i != ipandpred.end (); ++i)
                {
                  std::cout << "dest, " << i->first << ", pred is, " << i->second << ", ";
                  if (s_pred < i->second)
                    {
                      std::cout << std::endl;
                      // 宛先到達確率が高いノードにOLSRを用いてパケットを送信する。この時、modeは1のまま送信する。
                      std::cout << "(do_proposal2) time, " << Simulator::Now ().GetSeconds ()
                                << ", s_pred, " << s_pred << ", dest's pred, " << i->second
                                << ", is s_pred < dest's pred, "
                                << ", then send packet, " << packet->GetUid ()
                                << ", is send to the Node, " << convarray_olsraddr_node[i->first]
                                << ", (Address is, " << i->first << ", )" << std::endl;
                      olsr_testApp (packet, recv, sender, convarray_olsraddr_node[i->first], mode,
                                    uid, true);
                    }
                }
              std::cout << std::endl;
              std::cout << "(do_proposal3), queuelist[" << sender << "][" << uid << "* 10 + " << recv << "] = " << queuelist[sender][uid*10+recv] << std::endl;
              std::cout << "(do_proposal3), mem[" << sender << "][" << uid << "* 10 + " << recv << "] = " << mem[sender][uid*10+recv] << std::endl;
              if (queuelist[sender][uid * 10 + recv] == false &&
                  mem[sender][uid * 10 + recv] == false)
                {
                  // DTNでパケットを送信する
                  std::cout << "(do_proposal3) time, " << Simulator::Now ().GetSeconds () << ", "
                            << "do_dtntestapp" << std::endl;
                  dtn_testApp (packet, recv, sender, inter, mode, uid);
                }
            }
          break;
        }
    }
}

// 提案手法を実行する時にこの関数が使われる
void
do_proposal (Ptr<const Packet> packet, int sender, int recv, int inter, int uid, int mode)
{
  Ipv4Address table_addr[numberOfueNodes];
  olsrh.aaa (NodeList::GetNode (sender), table_addr);

  Ptr<Node> node = olsrContainer.Get (recv);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ipv4Address recvaddr = ipv4->GetAddress (1, 0).GetLocal ();

  // ノードが宛先ではない時
  if (sender != recv)
    {
      for (int i = 0; i < numberOfueNodes; i++)
        {
          // 宛先アドレスとOLSRの経路上のDESTアドレスが一致した時にOLSRモードを使ってパケットを宛先に対して送信する。
          if (recvaddr == table_addr[i])
            {
              mode = 0;
              olsr_testApp (packet, recv, sender, inter, mode, uid, false);
              mem[sender][uid * 10 + recv] = true;
              break;
            }
          // OLSRの経路上のDESTアドレスが”102.102.102.102”になった時、宛先アドレスがOLSRの経路上に存在しないと判断し、DTNを使用する。
          if (table_addr[i] == Ipv4Address ("102.102.102.102"))
            {
              mode = 1;
              std::map<Ipv4Address, double> ipandpred;

              // 経路表のIPアドレスと宛先到達確率を配列として抜き出す
              ipandpred = ExtractIpPred (NodeList::GetNode (sender), recv);
              // 自分の宛先到達確率も抜き出す
              double s_pred = mainmap[convarray[convarray_node_olsraddr[sender]]]
                                     [convarray[convarray_node_olsraddr[recv]]];
              std::cout << "(do_proposal) time, " << Simulator::Now ().GetSeconds () << ", recv, "
                        << recv << ", sender, " << sender << ", pred is, " << s_pred << ", ";

              // 経路表の宛先到達確率 > 自分の宛先到達確率となる、IPアドレスを持つノードを抽出する
              for (auto i = ipandpred.begin (); i != ipandpred.end (); ++i)
                {
                  std::cout << "dest, " << i->first << ", pred is, " << i->second << ", ";
                  if (s_pred < i->second)
                    {
                      std::cout << std::endl;
                      // 宛先到達確率が高いノードにOLSRを用いてパケットを送信する。この時、modeは1のまま送信する。
                      std::cout << "(do_proposal2) time, " << Simulator::Now ().GetSeconds ()
                                << ", s_pred, " << s_pred << ", dest's pred, " << i->second
                                << ", is s_pred < dest's pred, "
                                << ", then send packet, " << packet->GetUid ()
                                << ", is send to the Node, " << convarray_olsraddr_node[i->first]
                                << ", (Address is, " << i->first << ", )" << std::endl;
                      olsr_testApp (packet, recv, sender, convarray_olsraddr_node[i->first], mode,
                                    uid, true);
                    }
                }
              std::cout << std::endl;
              std::cout << "(do_proposal3), queuelist[" << sender << "][" << uid << "* 10 + " << recv << "] = " << queuelist[sender][uid*10+recv] << std::endl;
              std::cout << "(do_proposal3), mem[" << sender << "][" << uid << "* 10 + " << recv << "] = " << mem[sender][uid*10+recv] << std::endl;
              if (queuelist[sender][uid * 10 + recv] == false &&
                  mem[sender][uid * 10 + recv] == false)
                {
                  // DTNでパケットを送信する
                  std::cout << "(do_proposal3) time, " << Simulator::Now ().GetSeconds () << ", "
                            << "do_dtntestapp" << std::endl;
                  dtn_testApp (packet, recv, sender, inter, mode, uid);
                }
              break;
            }
        }
    }
}

/*
// OLSRの経路表が変わったときに提案手法を実行する関数であるdo_proposalを実行する関数
void
TraceTableChangedCallback (std::string context, uint32_t size)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  std::map<int, bool> queuelist2;
  std::map<int, bool> mem2;

  queuelist2 = queuelist[kym];
  mem2 = mem[kym];

  NS_LOG_INFO ("(TraceTableChangeCallback), " << Simulator::Now ().GetSeconds () << ", " << kym);

  for (auto itr = queuelist2.begin (); itr != queuelist2.end (); ++itr)
    {
      NS_LOG_INFO ("(TraceTableChangeCallback), " << Simulator::Now ().GetSeconds ()
                                                  << ", queuelist2[" << itr->first << "] is "
                                                  << itr->second);
    }

  NS_LOG_INFO ("(TracefTableChangeCallback), " << Simulator::Now ().GetSeconds ()
                                               << ", queuelist size is " << queuelist2.size ()
                                               << ", mem2 size is " << mem2.size ());

  if (queuelist2.size () > 0)
    {
      for (auto itr = queuelist2.begin (); itr != queuelist2.end (); ++itr)
        {
          if (itr->second == true)
            {
              int re_uid = itr->first / 10;
              int re_recv = itr->first % 10;
              if ((-1 < re_uid && re_uid < 8000000) && itr->first > 0 && re_uid > 0 && re_recv > 0)
                {
                  NS_LOG_INFO ("(TraceTableChangeCallback), "
                           << "itr->first is " << itr->first << ", re_uid is " << re_uid
                           << ", re_recv is " << re_recv);
                  if (mem2[itr->first] == false)
                    {
                      NS_LOG_INFO ("(TraceTableChangeCallback), "
                                   << "mem2[" << itr->first << "] is false");
                      Ptr<Packet> new_packet = Create<Packet> (packetSize);

                      int inter = 1;
                      int mode = 1;
                      IPv4TestTag tag;
                      tag.SetToken (re_uid * 100000 + re_recv * 1000 + inter * 10 + mode);
                      new_packet->AddPacketTag (tag);
                      do_proposal (new_packet, kym, re_recv, inter, re_uid, mode);
                    }
                  else
                    {
                      NS_LOG_INFO ("(TraceTableChangeCallback), "
                                   << "mem2[" << itr->first << "] is true");
                    }
                }
            }
        }
    }
}
*/

/*
*   以下、アプリケーションを実行する関数を記述
*/

void
olsr_testApp (Ptr<const Packet> packet, int recv, int sender, int inter, int mode, int uid,
              bool promode)
{
  int numPackets = 1;
  if (promode == false)
    {
      Address sinkAddress1 (InetSocketAddress (interfaces.GetAddress (recv), sinkPort));

      Ptr<Socket> ns3UdpSocket1;
      ns3UdpSocket1 = Socket::CreateSocket (olsrContainer.Get (sender),
                                            UdpSocketFactory::GetTypeId ()); //
      Ptr<MyApp> app1 = CreateObject<MyApp> ();
      app1->Setup (packet, ns3UdpSocket1, sinkAddress1, sender, recv, inter, packetSize, numPackets,
                   mode, uid, DataRate ("1Mbps"));
      app1->SetStartTime (Seconds (0.));
      app1->SetStopTime (Seconds (1));

      olsrContainer.Get (sender)->AddApplication (app1);
    }
  else
    {
      Address sinkAddress1 (InetSocketAddress (interfaces.GetAddress (inter), sinkPort));
      Ptr<Socket> ns3UdpSocket1;
      ns3UdpSocket1 = Socket::CreateSocket (olsrContainer.Get (sender),
                                            UdpSocketFactory::GetTypeId ()); //
      Ptr<MyApp> app1 = CreateObject<MyApp> ();
      app1->Setup (packet, ns3UdpSocket1, sinkAddress1, sender, recv, inter, packetSize, numPackets,
                   mode, uid, DataRate ("1Mbps"));
      app1->SetStartTime (Seconds (0.));
      app1->SetStopTime (Seconds (1));

      olsrContainer.Get (sender)->AddApplication (app1);
    }
}

void
dtn_testApp (Ptr<const Packet> packet, int recv, int sender, int inter, int mode, int uid)
{
  int numPackets = 1;
  // Create UDP application at n0
  Address sinkAddress2 (InetSocketAddress (interfaces2.GetAddress (recv),
                                           sinkPort)); // interface of n24
  Ptr<Socket> ns3UdpSocket2;
  ns3UdpSocket2 = Socket::CreateSocket (dtnContainer.Get (sender),
                                        UdpSocketFactory::GetTypeId ()); //
  Ptr<MyApp> app2 = CreateObject<MyApp> ();

  app2->Setup (packet, ns3UdpSocket2, sinkAddress2, sender, recv, inter, packetSize, numPackets,
               mode, uid, DataRate ("1Mbps"));
  app2->SetStartTime (Seconds (0.));
  app2->SetStopTime (Seconds (1));

  dtnContainer.Get (sender)->AddApplication (app2);
}

/*
* 以下、パケットを送信するための関数を記述
*/

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
      Ptr<MobilityModel> mobility = olsrContainer.Get (i)->GetObject<MobilityModel> ();
      //Vector current = mobility->GetPosition ();
      //NS_LOG_INFO (Simulator::Now ().GetSeconds ()
      //            << ", node " << i << "'s position is " << current);
    }
  NS_LOG_INFO (Simulator::Now ().GetSeconds ()
               << ", send_count, " << send_count.size () << ", recv_count, " << recv_count.size ());
}

/*
* 以下、IPアドレスやノードを変更するための配列を作るための関数を記述
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
      convarray_node_dtnaddr[i] = Ipv4Address (dtn_ipv4->GetAddress (1, 0).GetLocal ());
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


/*
* 以下、受信回数をトレースするトレースソースを記述
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

  uint64_t token = tagCopy.GetToken ();

  uint64_t uid = token / 100000;
  uint64_t temp = token - uid * 100000;
  uint64_t recv = temp / 1000;
  uint64_t temp2 = temp - recv * 1000;
  uint64_t inter = temp2 / 10;
  uint64_t mode = temp2 - inter * 10;

  if (kym >= numberOfueNodes)
    {
      kym = kym - numberOfueNodes;
    }

  if (queuelist[kym][uid * 10 + recv] == false && mem[kym][uid * 10 + recv] == false)
    {
      // DTNでパケットを送信する
      NS_LOG_INFO("(PacketSink/Rx1) time, " << Simulator::Now ().GetSeconds () << ", "
                   << "do_dtntestapp, " << "kym, " << kym << ", "
                   << "token, " << token << ", "
                   << "packet->GetUid(), " << packet->GetUid () << ", "
                   << "uid, " << uid << ", "
                   << "recv, " << recv << ", "
                   << "inter, " << inter << ", "
                   << "mode, " << mode);
      dtn_testApp (packet, recv, kym, inter, mode, uid);
      // queuelist[kym][uid * 10 + recv] = true;
    }

  if (kym == (int) recv)
    {
      NS_LOG_INFO ("(PacketSink/Rx2), "
                   << "Time, " << Simulator::Now ().GetSeconds () << ", "
                   << "Success At, "
                   << "kym, " << kym << ", "
                   << "packet->GetUid(), " << packet->GetUid () << ", "
                   << "uid, " << uid << ", "
                   << "recv, " << recv << ", "
                   << "inter, " << inter << ", "
                   << "mode, " << mode);
      recv_count[uid] = true;
    }
  else
    {
      NS_LOG_INFO ("(PacketSink/Rx3), "
                   << "Time, " << Simulator::Now ().GetSeconds () << ", "
                   << "Inter At, "
                   << "token, " << token << ", "
                   << "kym, " << kym << ", "
                   << "packet->GetUid(), " << packet->GetUid () << ", "
                   << "uid, " << uid << ", "
                   << "recv, " << recv << ", "
                   << "inter, " << inter << ", "
                   << "mode, " << mode);
    }
}

/*
* 以下、パケット損失をトレースするトレースソースを記述
*/

void
UdpL4ProtocolDrop (std::string context, Ptr<const Packet> packet)
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

  uint64_t token = tagCopy.GetToken ();

  uint64_t uid = token / 100000;

  NS_LOG_INFO ("(UdpL4Protocol/Drop), " << Simulator::Now ().GetSeconds () << ", " << kym << ", "
                                        << packet->GetUid () << ", " << uid << ", "
                                        << packet->GetSize ());
}

void
ArpL3ProtocolDrop (std::string context, Ptr<const Packet> packet)
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

  uint64_t token = tagCopy.GetToken ();

  uint64_t uid = token / 100000;

  NS_LOG_INFO ("(ArpL3Protocol/Drop)" << Simulator::Now ().GetSeconds () << ", " << kym << ", "
                                      << packet->GetUid () << ", " << uid << ", "
                                      << packet->GetSize ());
}

void
Ipv4L3Drop (std::string context, const Ipv4Header &header, Ptr<const Packet> packet,
            Ipv4L3Protocol::DropReason reason, Ptr<Ipv4> ipv4, uint32_t interface)
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

  uint64_t token = tagCopy.GetToken ();

  uint64_t uid = token / 100000;

  NS_LOG_INFO ("(Ipv4L3Protocol/Drop), " << Simulator::Now ().GetSeconds () << ", " << kym << ", "
                                         << packet->GetUid () << ", " << uid << ", "
                                         << packet->GetSize () << ", " << reason);
}

void
ArpCacheDrop (std::string context, Ptr<const Packet> packet)
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

  uint64_t token = tagCopy.GetToken ();

  uint64_t uid = token / 100000;

  NS_LOG_INFO ("(ArpCache/Drop), " << Simulator::Now ().GetSeconds () << ", " << kym << ", "
                                   << packet->GetUid () << ", " << uid << ", "
                                   << packet->GetSize ());
}

struct mob_list
{ // zahyou no hensu
  int node_no;
  double waypoint_t;
  double waypoint_x;
  double waypoint_y;
  double waypoint_z;
};

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  struct mob_list wpl[pre_numberOfvehicles][1000] = {}; //
  int jc[pre_numberOfvehicles] = {};
  int i = 0;

  //Time::SetResolution (Time::NS);
  //LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable("EpidemicRoutingProtocol", LOG_LEVEL_WARN);
  LogComponentEnable("EpidemicPacketQueue", LOG_LEVEL_WARN);
  LogComponentEnable ("test-olsropp", LOG_LEVEL_INFO);
  //LogComponentEnable("ArpL3Protocol", LOG_LEVEL_LOGIC);
  //LogComponentEnable ("Queue", LOG_LEVEL_LOGIC);

  std::ifstream ifss ("./random-waypoint-mobility-2-2.mob"); 
  std::string str;
  if (ifss.fail ())
    {
      std::cerr << "失敗" << std::endl;
      return -1;
    }

  while (std::getline (ifss, str))
    {
      std::string delim ("= :");
      std::list<std::string> list_waypointstr;
      boost::split (list_waypointstr, str, boost::is_any_of (delim));
      auto itr2 = list_waypointstr.begin ();
      ++itr2; //waypointtime
      ++itr2; //node
      ++itr2; //nodeno
      i = std::stoi (*itr2);
      //std::cout << "i is " << i << std::endl;
      wpl[i][jc[i]].node_no = std::stoi (*itr2);
      --itr2; //node
      --itr2; //waypointtime
      std::string ytk = *itr2;
      ytk.erase (--ytk.end ());
      ytk.erase (--ytk.end ());
      wpl[i][jc[i]].waypoint_t = std::stod (ytk);
      ++itr2; //node
      ++itr2; //nodeno
      ++itr2; //pos
      ++itr2; //x
      wpl[i][jc[i]].waypoint_x = std::stod (*itr2);
      ++itr2; //y
      wpl[i][jc[i]].waypoint_y = std::stod (*itr2);
      ++itr2; //z
      wpl[i][jc[i]].waypoint_z = std::stod (*itr2);
      jc[i]++;
    }

  olsrContainer.Create (numberOfueNodes);
  dtnContainer.Create (numberOfueNodes);

  /*
   *      Mobility model Setup
   */

  MobilityHelper mobility;
  MobilityHelper mobility2;

  Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> initialAlloc2 = CreateObject<ListPositionAllocator> ();

  initialAlloc->Add (Vector (500, 500, 200));
  initialAlloc->Add (Vector (1500, 500, 200));
  initialAlloc->Add (Vector (2500, 500, 200));
  initialAlloc->Add (Vector (500, 1500, 200));
  initialAlloc->Add (Vector (1500, 1500, 200));
  initialAlloc->Add (Vector (2500, 1500, 200));
  initialAlloc2->Add (Vector (200, 1300, 1.5));
  //initialAlloc2->Add (Vector (1500, 1500, 1.5));
  initialAlloc2->Add (Vector (1500, 500, 1.5));
  initialAlloc2->Add (Vector (2500, 1500, 1.5));

  mobility.SetPositionAllocator (initialAlloc);
  mobility2.SetPositionAllocator (initialAlloc2);

  mobility.SetMobilityModel ("ns3::WaypointMobilityModel");
  //mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  for (int i = 0; i <= 5; i++)
    {
      mobility.Install (olsrContainer.Get (i));
      mobility.Install (dtnContainer.Get (i));
    }

  for (int i = 0; i <= 5; i++)
    {
      Ptr<WaypointMobilityModel> uav_waypoint =
          DynamicCast<WaypointMobilityModel> (olsrContainer.Get (i)->GetObject<MobilityModel> ());

      Ptr<WaypointMobilityModel> uav_waypoint2 =
          DynamicCast<WaypointMobilityModel> (dtnContainer.Get (i)->GetObject<MobilityModel> ());

      uav_waypoint->AddWaypoint (
          Waypoint (Seconds (0), Vector (i % 3 * 1000 + 500, i / 3 * 1000 + 500, 200)));

      uav_waypoint2->AddWaypoint (
          Waypoint (Seconds (0), Vector (i % 3 * 1000 + 500, i / 3 * 1000 + 500, 200)));
    }

  mobility2.SetMobilityModel ("ns3::WaypointMobilityModel");

  for (int i = 6; i <= 8; i++)
    {
      mobility2.Install (olsrContainer.Get (i));
      mobility2.Install (dtnContainer.Get (i));
    }

  for (int u = numberOfvehicles; u < numberOfvehicles + pre_numberOfvehicles; u++)
    {
      Ptr<WaypointMobilityModel> waypoint =
          DynamicCast<WaypointMobilityModel> (olsrContainer.Get (u)->GetObject<MobilityModel> ());

      Ptr<WaypointMobilityModel> waypoint2 =
          DynamicCast<WaypointMobilityModel> (dtnContainer.Get (u)->GetObject<MobilityModel> ());

      //NS_LOG_INFO ("u-pre_numberOfvehicles is " << u - numberOfvehicles);

      for (int v = 0; v < jc[u - numberOfvehicles]; v++)
        {
          std::cout << "wpl[" <<u - numberOfvehicles << "][" << v << "] is " <<wpl[u - numberOfvehicles][v].waypoint_t<< std::endl;
          waypoint->AddWaypoint (Waypoint (NanoSeconds (wpl[u - numberOfvehicles][v].waypoint_t),
                                           Vector (wpl[u - numberOfvehicles][v].waypoint_x,
                                                   wpl[u - numberOfvehicles][v].waypoint_y, 1.5)));
          waypoint2->AddWaypoint (Waypoint (NanoSeconds (wpl[u - numberOfvehicles][v].waypoint_t),
                                            Vector (wpl[u - numberOfvehicles][v].waypoint_x,
                                                    wpl[u - numberOfvehicles][v].waypoint_y, 1.5)));
        }
    }

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

  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode),
                                "ControlMode", StringValue (ControlMode));

  devices = wifi.Install (wifiPhy, wifiMac, olsrContainer);
  devices2 = wifi.Install (wifiPhy, wifiMac, dtnContainer);

  /*
  *    Routing Setup
  */

  uint32_t epidemicHopCount = 1000;
  uint32_t epidemicQueueLength = 1000;
  Time epidemicQueueEntryExpireTime = Seconds (1000);
  Time epidemicBeaconInterval = Seconds (0.5);

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
      Simulator::Schedule (Seconds (i), &sendpacket_interval);
    }

  /*
  *   others
  */
  //AsciiTraceHelper ascii;
  //wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("test-tracesource.tr"));
  //wifiPhy.EnablePcapAll ("test-proposal-0815");

  Simulator::Stop (Seconds (appTotalTime));

  AnimationInterface anim ("jikken-proposal-0915.xml");
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
  //Config::Connect ("NodeList/*/$ns3::Ipv4L3Protocol/Drop", MakeCallback(&Ipv4L3Drop));
  //Config::Connect ("NodeList/*/$ns3::Ipv4L3Protocol/Rx", MakeCallback (&Rx));
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
  Config::Connect ("NodeList/*/$ns3::Epidemic::RoutingProtocol/Tracepastcontact",
                   MakeCallback (&Tracepastcontact));
  Config::Connect ("NodeList/*/$ns3::Epidemic::RoutingProtocol/TraceJudgeFrompastcontact",
                   MakeCallback (&TraceJudgeFrompastcontact));
  Config::Connect ("NodeList/*/$ns3::olsr::RoutingProtocol/TraceAddEntry",
                   MakeCallback (&TraceAddEntry));
  Config::Connect ("NodeList/*/$ns3::ArpL3Protocol/CacheList/*/Drop", MakeCallback (&ArpCacheDrop));
  Config::Connect ("NodeList/*/$ns3::UdpL4Protocol/SocketList/*/Drop",
                   MakeCallback (&UdpL4ProtocolDrop));
  Config::Connect ("NodeList/*/$ns3::ArpL3Protocol/Drop", MakeCallback (&ArpL3ProtocolDrop));
  //Config::Connect("NodeList/*/$ns3::olsr::RoutingProtocol/RoutingTableChanged", MakeCallback(&TraceTableChangedCallback));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}