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
uint32_t packetSize = 2048; // 2048
double appTotalTime = 125; // simulator stop time
double appDataStart = 0.0;
double appDataEnd = 125;
int numberOfueNodes = 4;
int numPackets = 1;
uint16_t sinkPort = 6; // use the same for all apps

// mainaddress, neighboraddress, prediction
std::map< Ipv4Address, std::map<Ipv4Address, double> > mainmap;

// prototype sengen
void olsr_or_dtn (int sender);
void dtn_to_olsr (int sender, int recv, int uid);
void olsr_testApp (int recv, int sender, int mode, int uid);
void dtn_testApp (int recv, int sender, int mode, int uid);


// From the UID and the sender in tag, 
// check whether the source is sending packet with a specific UID or not.
std::map<int, bool> mem;

/*
*     MyTag Class
*/

class MyTag : public Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  // these are our accessors to our tag structure
  void SetSimpleValue (uint8_t value);
  uint8_t GetSimpleValue (void) const;

private:
  uint8_t m_simpleValue;
};

TypeId
MyTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyTag")
                          .SetParent<Tag> ()
                          .AddConstructor<MyTag> ()
                          .AddAttribute ("SimpleValue", "A simple value", EmptyAttributeValue (),
                                         MakeUintegerAccessor (&MyTag::GetSimpleValue),
                                         MakeUintegerChecker<uint8_t> ());
  return tid;
}
TypeId
MyTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
MyTag::GetSerializedSize (void) const
{
  return 1;
}
void
MyTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_simpleValue);
}
void
MyTag::Deserialize (TagBuffer i)
{
  m_simpleValue = i.ReadU8 ();
}
void
MyTag::Print (std::ostream &os) const
{
  os << "v=" << (uint32_t) m_simpleValue;
}
void
MyTag::SetSimpleValue (uint8_t value)
{
  m_simpleValue = value;
}
uint8_t
MyTag::GetSimpleValue (void) const
{
  return m_simpleValue;
}

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
      m_uid(-1)
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
  if (mem[m_uid*10+m_sender] == false) // if this node has never sent a packet 
  {
    IPv4TestTag tag;
    Ptr<Packet> packet = Create<Packet> (m_packetSize);

    if (m_uid == -1) // This packet is generated by sender
    {
        tag.SetToken (packet->GetUid () * 10000 + m_receiver * 10 + m_mode);
        packet->AddPacketTag (tag);
        m_socket->Send (packet);
        mem[m_uid*10+m_sender] = true;
    }
    else // This packet is generated by intermediary node
    { 
        tag.SetToken (m_uid * 10000 + m_receiver * 10 + m_mode);
        packet->AddPacketTag (tag);
        m_socket->Send (packet);
        mem[m_uid*10+m_sender] = true;
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

/*
*    tracesource list
*/

/// MonitorSnifferRx
void
MonitorSnifferRx (std::string context, Ptr<const Packet> packet, uint16_t channelFreqMhz,
                  WifiTxVector txVector, MpduInfo aMpdu, SignalNoiseDbm signalNoise)
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


      IPv4TestTag tagCopy;
      packet->PeekPacketTag (tagCopy);

      int token = tagCopy.GetToken ();

      int uid = token / 10000;
      int temp = token - uid * 10000;
      int recv = temp / 10;
      int mode = temp - recv * 10;

      if (kym == recv || kym == recv + numberOfueNodes)
        {
          NS_LOG_INFO ( "(MonitorSnifferRx) "
                    << "Time:" << Simulator::Now().GetSeconds() << ", "
                    << "Success At "
                    << "packet->GetUid(): " << packet->GetUid() << ", "
                    << "uid:" << uid << ", "
                    << "recv:" << kym << ", "
                    << "mode:" << mode);
        }
      else
        {
          if (mode == 1 && kym < recv) // dtn mode
            {
              int time = Simulator::Now ().GetSeconds ();
              int endtime = (int) appDataEnd - time;
              for (int i = 0; i < endtime; i++)
                {
                  Simulator::Schedule (Seconds (i), &dtn_to_olsr, kym, recv, uid);
                }
            }
        }
    }
}

void 
TraceSendPacket (std::string context, Ptr<const Packet> packet, Ipv4Address dest)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo
  
  int recvnodenum = -1;
  for (int i = 0; i < numberOfueNodes; i++)
  {
    Ptr<Node> node = dtnContainer.Get(i);
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal ();
    if (dest == addr)
    {
     recvnodenum = i; 
     break;
    }
  }
  Ptr<Node> node = olsrContainer.Get (recvnodenum);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ipv4Address recvaddr = ipv4->GetAddress (1, 0).GetLocal ();

  bool judge;
  Ipv4Address table_addr[10];
  olsrh.aaa(NodeList::GetNode(kym-numberOfueNodes), table_addr);
  for (int i = 0; i < 10; i++)
  {
    if(recvaddr == table_addr[i])
    {
      judge = true;
      epidemic.SendJudge(dtnContainer, kym-numberOfueNodes, packet, judge);
      break;
    }
    if (table_addr[i] == Ipv4Address ("102.102.102.102")) 
    {
      judge = false;
      epidemic.SendJudge(dtnContainer, kym-numberOfueNodes, packet, judge);
      break;
    }
  }
}

void TraceupdateDeliveryPredFor(std::string context, Ipv4Address mainAddress, Ipv4Address addr, double newValue)
{
  mainmap[mainAddress][addr] = newValue;
}

void TraceupdateTransitivePreds(std::string context, Ipv4Address addr)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  epidemic.setOpp_preds(dtnContainer, kym-numberOfueNodes, mainmap[addr]);
}

void TraceSendPacketFromQueue (std::string context, Ipv4Address dst)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  epidemic.setOpp_preds2(dtnContainer, kym-numberOfueNodes, mainmap[dst]);
}

/*
*   execute application
*/

void
olsr_or_dtn (int sender)
{
  int recv = 0;
  int mode = 0;
  int uid = -1;

  Ipv4Address table_addr[10];
  olsrh.aaa (NodeList::GetNode (sender), table_addr);

  std::random_device rd{};

  while (1)
    {
      recv = rd () % numberOfueNodes;
      if (recv != sender)
        {
          break;
        }
    }

  recv = 3;

  Ptr<Node> node = olsrContainer.Get (recv);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ipv4Address recvaddr = ipv4->GetAddress (1, 0).GetLocal ();

  for (int i = 0; i < 10; i++)
    {
      if (recvaddr == table_addr[i])
        {
          mode = 0;
          olsr_testApp (recv, sender, mode, uid);
          break;
        }
      if (table_addr[i] == Ipv4Address ("102.102.102.102"))
        {
          mode = 1;
          dtn_testApp (recv, sender, mode, uid);
          break;
        }
    }
}

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
      if (recvaddr == table_addr[i] && mem[uid*10+sender] == false)
        {
          int mode = 0;
          epidemic.Drop(dtnContainer, uid);
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

  app1->Setup (ns3UdpSocket1, sinkAddress1, sender, recv, packetSize, numPackets, mode, uid, DataRate ("1Mbps"));
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

  app2->Setup (ns3UdpSocket2, sinkAddress2, sender, recv, packetSize, numPackets, mode, uid, DataRate ("1Mbps"));
  app2->SetStartTime (Seconds (0.));
  app2->SetStopTime (Seconds (1));

  dtnContainer.Get (sender)->AddApplication (app2);
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
  initialAlloc->Add (Vector (0, 0, 1.5));
  initialAlloc->Add (Vector (250, 0, 1.5));
  initialAlloc->Add (Vector (500, 0, 1.5));
  initialAlloc->Add (Vector (800, 0, 1.5));
  mobility.SetPositionAllocator (initialAlloc);

  //mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetMobilityModel ("ns3::WaypointMobilityModel");
  mobility.Install (olsrContainer);
  mobility.Install (dtnContainer);

  Ptr<WaypointMobilityModel> waypoint =
    DynamicCast<WaypointMobilityModel> (olsrContainer.Get (3)->GetObject<MobilityModel> ());
  waypoint->AddWaypoint (Waypoint(Seconds(55), Vector(800, 0, 1.5)));
  waypoint->AddWaypoint (Waypoint(Seconds(100), Vector(750, 0, 1.5)));

  Ptr<WaypointMobilityModel> waypoint2 =
    DynamicCast<WaypointMobilityModel> (dtnContainer.Get (3)->GetObject<MobilityModel> ());
  waypoint2->AddWaypoint (Waypoint(Seconds(55), Vector(800, 0, 1.5)));
  waypoint2->AddWaypoint (Waypoint(Seconds(100), Vector(750, 0, 1.5)));

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

  uint32_t sender = 0;
  Time timer = Seconds (50);
  Simulator::Schedule (Seconds (51), &olsr_or_dtn, sender);

  /*
  *   others
  */
  AsciiTraceHelper ascii;
  wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("test-tracesource.tr"));
  wifiPhy.EnablePcapAll ("test-tracesource");

  Simulator::Stop (Seconds (appTotalTime));

  AnimationInterface anim ("test-olsropp-animation.xml");
  anim.EnablePacketMetadata (true);
  anim.EnableIpv4RouteTracking ("tinkoko.xml", Seconds (0), Seconds (appDataEnd), Seconds (0.25));

  /*
   *  tracing olsr routingtable
   */
  //list.PrintRoutingTableAt(timer, olsrContainer.Get(1), streamer);

  // Mandatory
  //Config::Connect ("NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRxDrop", MakeCallback (&MacRxDrop));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTxDrop", MakeCallback (&MacTxDrop));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback (&PhyTxDrop));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback (&PhyRxDrop));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxBegin", MakeCallback (&PhyTxBegin));
  //Config::Connect ("NodeList/*/$ns3::Ipv4L3Protocol/Drop", MakeCallback(&Drop));
  //Config::Connect ("NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback (&ReceivedPacket));
  //Config::Connect("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/State/RxError", MakeCallback(&RxError));
  Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx",
                   MakeCallback (&MonitorSnifferRx));
  Config::Connect ("NodeList/*/$ns3::Epidemic::RoutingProtocol/TraceSendPacket",
                   MakeCallback (&TraceSendPacket));
  Config::Connect ("NodeList/*/$ns3::Epidemic::RoutingProtocol/TraceupdateDeliveryPredFor",
                   MakeCallback (&TraceupdateDeliveryPredFor));
  Config::Connect ("NodeList/*/$ns3::Epidemic::RoutingProtocol/TraceupdateTransitivePreds",
                   MakeCallback (&TraceupdateTransitivePreds));
  Config::Connect ("NodeList/*/$ns3::Epidemic::RoutingProtocol/TracesendPacketFromQueue",
                   MakeCallback (&TraceSendPacketFromQueue));
  Simulator::Run ();
  Simulator::Destroy ();
  
  return 0;
}