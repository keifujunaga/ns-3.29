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

// NetAnim
#include "ns3/netanim-module.h"

// Others
#include <boost/algorithm/string.hpp>
#include <vector>
#include <random>
#include <fstream>
#include <string>
#include <array>

#define R 100

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

NodeContainer olsrContainer;
NodeContainer dtnContainer;
NetDeviceContainer devices;
NetDeviceContainer devices2;
Ipv4InterfaceContainer interfaces;
Ipv4InterfaceContainer interfaces2;
OlsrHelper olsrh;
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
//int recv;
//int sender;
//int receiver[R];
//int uid;
bool olsrdtn[100000][30];

// prototype sengen
void fileopentest (std::string filename);
void fileopentest2 (std::string filename, int kym, int uid);
void check_sendrecvlist (int sender, int receiver[]);
void olsr_or_dtn (int sender, int receiver[]);
void olsr_or_dtn2 (int sender, int receiver[], int uid);
void kara ();
void olsr_testApp (int recv, int sender, int mode);
void dtn_testApp (int recv, int sender, int mode);

// typedef
typedef std::map<std::pair<Ipv4Address, Ipv4Address>, std::vector<Ipv4Address>> Tables;
typedef std::map<std::pair<Ipv4Address, Ipv4Address>, Ipv4Address> NextHops;
Tables tables;
NextHops next_hops;

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
              uint32_t packetSize, uint32_t nPackets, uint32_t mode, DataRate dataRate);
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
  uint32_t m_senderAddress;
  uint32_t m_receiverAddress;
  uint32_t m_mode;
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
      m_senderAddress (0),
      m_receiverAddress (0),
      m_mode (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t senderAddress, uint32_t receiverAdress,
              uint32_t packetSize, uint32_t nPackets, uint32_t mode, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_senderAddress = senderAddress;
  m_receiverAddress = receiverAdress;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_mode = mode;
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
  IPv4TestTag tag;
  Ptr<Packet> packet = Create<Packet> (m_packetSize);

  std::cout << "(MyApp) "
            << "packet uid is " << packet->GetUid () << std::endl;
  std::cout << "(MyApp) "
            << "packet recvadd is " << m_receiverAddress << std::endl;
  std::cout << "(MyApp) "
            << "mode is " << m_mode << std::endl;

  tag.SetToken (packet->GetUid () * 10000 + m_receiverAddress * 10 + m_mode);
  packet->AddPacketTag (tag);

  m_socket->Send (packet);

  NS_LOG_UNCOND ("(MyApp) " << m_senderAddress << ", " << m_receiverAddress << ", "
                            << packet->GetUid () << ", " << packet->GetSize ());

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

void
CourseChange (std::string context, Ptr<const MobilityModel> model)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo
  Vector position = model->GetPosition ();

  NS_LOG_UNCOND ("Simulation time " << Simulator::Now ().GetSeconds ());
  NS_LOG_UNCOND ("node = " << kym << ", x = " << position.x << ", y = " << position.y
                           << ", z = " << position.z);
}

/// WifiMac
void
MacRxDrop (std::string context, Ptr<const Packet> packet)
{
  NS_LOG_UNCOND ("(MacRxDrop) packet uid is " << packet->GetUid ());
}

void
MacTxDrop (std::string context, Ptr<const Packet> packet)
{
  NS_LOG_UNCOND ("((MacTxDrop) packet uid is " << packet->GetUid ());
}

/// WifiPhy
void
PhyTxBegin (std::string context, Ptr<const Packet> packet)
{
  if (packet->GetSize () == 2048 || packet->GetSize () == 2112)
    {
      std::string delim ("/");
      std::list<std::string> list_string;
      boost::split (list_string, context, boost::is_any_of (delim));
      auto itr = list_string.begin ();
      ++itr;
      ++itr;
      int kym = std::stoi (*itr); // NodeNo
      NS_LOG_UNCOND ("(PhyTxBegin) " << kym << ", " << packet->GetUid () << ", "
                                     << packet->GetSize ());
      if (kym == 0)
        {
          NS_LOG_UNCOND ("(PhyTxBegin) " << kym << ", " << packet->GetUid () << ", "
                                         << packet->GetSize ());
        }
    }
}

void
PhyTxDrop (std::string context, Ptr<const Packet> packet)
{
  NS_LOG_UNCOND ("(PhyTxDrop) packet uid is " << packet->GetUid ());
}

int packetdropcounter;

void
PhyRxDrop (std::string context, Ptr<const Packet> packet)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  NS_LOG_UNCOND ("(PhyRxDrop) packet uid is " << packet->GetUid () << " packet size is "
                                              << packet->GetSize () << " node is " << kym);
  if (packet->GetSize () == 2112 && kym == 2)
    {
      packetdropcounter += 2048;
    }
  NS_LOG_UNCOND ("packetdropcounter is " << packetdropcounter);
}

/// Ipv4L3Protocol
void
Drop (std::string context, const Ipv4Header &header, Ptr<const Packet> packet,
      Ipv4L3Protocol::DropReason reason, Ptr<Ipv4> ipv4, uint32_t interface)
{
  NS_LOG_UNCOND ("tionko");
}

///WifiPhyStateHelper
void
RxError (std::string context, Ptr<const Packet> packet, double nanikore)
{
  NS_LOG_UNCOND ("(RxError) packet uid is " << packet->GetUid ());
}

/// PacketSink
void
ReceivedPacket (std::string context, Ptr<const Packet> packet, const Address &)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  NS_LOG_UNCOND ("(ReceivedPacket) " << kym << ", " << packet->GetUid () << ", "
                                     << packet->GetSize ());
}


/// MonitorSnifferRx
void
MonitorSnifferRx (std::string context, Ptr<const Packet> packet, uint16_t channelFreqMhz,
                  WifiTxVector txVector, MpduInfo aMpdu, SignalNoiseDbm signalNoise)
{
  if (packet->GetSize () >= 2048)
    {
      std::cout << "(MonitorSnifferRx) " << Simulator::Now ().GetSeconds () << context << std::endl;
      std::string delim ("/");
      std::list<std::string> list_string;
      boost::split (list_string, context, boost::is_any_of (delim));
      auto itr = list_string.begin ();
      ++itr;
      ++itr;
      int kym = std::stoi (*itr); // NodeNo

      int time = Simulator::Now ().GetSeconds ();
      std::cout << "(MonitorSnifferRx) time is " << time << "s" << std::endl;
      //olsrh.printolsrroutingtable(olsrContainer, Simulator::Now());
      std::cout << "(MonitorSnifferRx) PrintRoutingtable At " << time << "s" << std::endl;

      IPv4TestTag tagCopy;
      packet->PeekPacketTag (tagCopy);

      // std::cout << "(MonitorSnifferRx) " << tagCopy.GetToken() << std::endl;

      int token = tagCopy.GetToken ();

      int uid = token / 10000;
      int temp = token - uid * 10000;
      int recvadd = temp / 10;
      int mode = temp - recvadd * 10;

      if (mode == 1)
        {
          std::cout << "node " << kym << " is using dtn" << std::endl;
          for (int i = time; i < (int) appDataEnd; i++)
            {
              if (olsrdtn[uid][kym + 1])
                {
                  std::cout << "olsrdtn[" << uid << "][" << kym + 1 << "] is "
                            << olsrdtn[uid][kym + 1] << std::endl;
                  Simulator::Schedule (Seconds (time), &fileopentest2, "tinko2.txt", kym + 1, uid);
                }
            }
        }

      /*
      IPv4TestTag tagCopy;
      packet->PeekPacketTag (tagCopy);

      // std::cout << "(MonitorSnifferRx) " << tagCopy.GetToken() << std::endl;

      int token = tagCopy.GetToken ();

      int uid = token / 10000;
      int temp = token - uid * 10000;
      int recvadd = temp / 10;
      int mode = temp - recvadd * 10;
      */
      /*
      if (kym == recvadd - 1)
        {

          std::cout << "(MonitorSnifferRx) " << recvadd << ", " << uid << ", " << packet->GetSize ()
                    << ", " << mode << std::endl;

        }
           */

      std::cout << "(MonitorSnifferRx) " << Simulator::Now ().GetSeconds () << ", " << kym + 1
                << ", " << packet->GetSize () << std::endl;
    }
}

/*
*   execute application
*/
void
fileopentest (std::string filename)
{
  std::ifstream ifs (filename);
  std::string str;
  std::string delim ("		.");
  std::string test (": ,");
  std::list<std::string> list_string;
  std::string temp;
  int sender = 0;
  int receiver[R] = {0};
  int count = 0;
  int count2 = 0;

  if (ifs.fail ())
    {
      std::cerr << "Failed to open file." << std::endl;
    }
  while (getline (ifs, str))
    {
      if (count == 0)
        {
          boost::split (list_string, str, boost::is_any_of (test));
          auto itr = list_string.begin ();
          itr++;
          itr++;
          std::cout << "itr is " << *itr << std::endl;
          sender = std::stoi (*itr);
          sender++;
          count++;
        }
      boost::split (list_string, str, boost::is_any_of (delim));
      auto itr = list_string.begin ();
      if (*itr == "192" || *itr == "127")
        {
          if (*itr == "127")
            {
              break;
            }
          ++itr;
          ++itr;
          ++itr;
          std::cout << "itr is " << *itr << std::endl;
          receiver[count2] = std::stoi (*itr);
          count2++;
        }
    }

  check_sendrecvlist (sender, receiver);
  olsr_or_dtn (sender, receiver);
}

void
fileopentest2 (std::string filename, int kym, int uid)
{
  std::ifstream ifs (filename);
  std::string str;
  std::string delim ("		.");
  std::string test (": ,");
  std::list<std::string> list_string;
  std::string temp;
  int sender = kym;
  int receiver[R] = {0};
  int count = 0;
  int count2 = 0;

  if (ifs.fail ())
    {
      std::cerr << "Failed to open file." << std::endl;
    }
  while (getline (ifs, str))
    {
      if (count == 0)
        {
          boost::split (list_string, str, boost::is_any_of (test));
          auto itr = list_string.begin ();
          itr++;
          itr++;
          std::cout << "itr is " << *itr << std::endl;
          sender = std::stoi (*itr);
          sender++;
          count++;
        }
      boost::split (list_string, str, boost::is_any_of (delim));
      auto itr = list_string.begin ();
      if (*itr == "192" || *itr == "127")
        {
          if (*itr == "127")
            {
              break;
            }
          ++itr;
          ++itr;
          ++itr;
          std::cout << "itr is " << *itr << std::endl;
          receiver[count2] = std::stoi (*itr);
          count2++;
        }
    }

  check_sendrecvlist (sender, receiver);
  olsr_or_dtn2 (sender, receiver, uid);
}

void
check_sendrecvlist (int sender, int receiver[])
{
  std::cout << "sender is " << sender << std::endl;
  for (int i = 0; i < R; i++)
    {
      if (receiver[i] == 0)
        break;
      std::cout << "receiver is " << receiver[i] << std::endl;
    }
}

void
olsr_or_dtn (int sender, int receiver[])
{
  int recv = 0;
  int mode = 0;

  std::random_device rd{};

  while (1)
    {
      recv = rd () % numberOfueNodes + 1;
      if (recv != sender)
        {
          break;
        }
    }

  std::cout << "recv is " << recv << std::endl;

  for (int i = 0; i < R; i++)
    {
      std::cout << "receiver[" << i << "] (olsr_to_dtn) is " << receiver[i] << std::endl;
      if (receiver[i] <= recv)
        {
          if (recv == receiver[i])
            {
              std::cout << "send by olsr" << std::endl;
              mode = 0;
              olsr_testApp (recv, sender, mode);
              break;
            }
        }

      if (receiver[i] > recv || receiver[i] == 0)
        {
          std::cout << "send by dtn" << std::endl;
          mode = 1;
          dtn_testApp (recv, sender, mode);
          break;
        }
    }
}

void
olsr_or_dtn2 (int sender, int receiver[], int uid)
{
  int recv = 0;
  int mode = 0;

  std::random_device rd{};

  while (1)
    {
      recv = rd () % numberOfueNodes + 1;
      if (recv != sender)
        {
          break;
        }
    }

  std::cout << "recv is " << recv << std::endl;

  for (int i = 0; i < R; i++)
    {
      std::cout << "receiver[" << i << "] (olsr_to_dtn) is " << receiver[i] << std::endl;
      if (receiver[i] <= recv)
        {
          if (recv == receiver[i])
            {
              std::cout << "send by olsr" << std::endl;
              mode = 0;
              olsrdtn[uid][sender] = false;
              olsr_testApp (recv, sender, mode);
              break;
            }
        }

      if (receiver[i] > recv || receiver[i] == 0)
        {
          std::cout << "send by dtn" << std::endl;
          mode = 1;
          olsrdtn[uid][sender] = true;
          dtn_testApp (recv, sender, mode);
          break;
        }
    }
}

void
kara ()
{
  system ("echo -n > tinko.txt");
}

void
dtn_testApp (int recv, int sender, int mode)
{
  int numPackets = 1;
  // Create UDP application at n0
  Address sinkAddress2 (InetSocketAddress (interfaces2.GetAddress (recv - 1),
                                           sinkPort)); // interface of n24
  Ptr<Socket> ns3UdpSocket2;
  ns3UdpSocket2 = Socket::CreateSocket (dtnContainer.Get (sender - 1),
                                        UdpSocketFactory::GetTypeId ()); //
  Ptr<MyApp> app2 = CreateObject<MyApp> ();

  app2->Setup (ns3UdpSocket2, sinkAddress2, sender, recv, packetSize, numPackets, mode,
               DataRate ("1Mbps"));
  app2->SetStartTime (Seconds (0.));
  app2->SetStopTime (Seconds (1));

  dtnContainer.Get (sender - 1)->AddApplication (app2);
}

void
olsr_testApp (int recv, int sender, int mode)
{
  int numPackets = 1;
  // Create UDP application at n0
  Address sinkAddress1 (InetSocketAddress (interfaces.GetAddress (recv - 1),
                                           sinkPort)); // interface of n24
  Ptr<Socket> ns3UdpSocket1;
  ns3UdpSocket1 = Socket::CreateSocket (olsrContainer.Get (sender - 1),
                                        UdpSocketFactory::GetTypeId ()); //
  Ptr<MyApp> app1 = CreateObject<MyApp> ();

  app1->Setup (ns3UdpSocket1, sinkAddress1, sender, recv, packetSize, numPackets, mode,
               DataRate ("1Mbps"));
  app1->SetStartTime (Seconds (0.));
  app1->SetStopTime (Seconds (1));

  olsrContainer.Get (sender - 1)->AddApplication (app1);
}

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Time::SetResolution (Time::NS);
  //LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);

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
  initialAlloc->Add (Vector (750, 0, 1.5));

  mobility.SetPositionAllocator (initialAlloc);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (olsrContainer);
  mobility.Install (dtnContainer);

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

  EpidemicHelper epidemic;
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

  //int recv = 3;
  //int sender = 1;
  //int mode = 1;
  //int numPackets = 1;

  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory",
                                     InetSocketAddress (Ipv4Address::GetAny (), sinkPort));

  ApplicationContainer sinkApps2;
  sinkApps2 = packetSinkHelper.Install (olsrContainer); //n2 as sink
  sinkApps2 = packetSinkHelper.Install (dtnContainer); //n2 as sink
  sinkApps2.Start (Seconds (0.));
  sinkApps2.Stop (Seconds (1000.));

  /*
  olsr_testApp(4, 1, 0);
  olsr_testApp(4, 1, 0);
  dtn_testApp(4, 1, 0);
  dtn_testApp(4, 1, 0);
  */

  /*
  Ptr<Socket> ns3UdpSocket3 = Socket::CreateSocket (dtnContainer.Get (sender),
                                                    UdpSocketFactory::GetTypeId ()); //source at n0

  app[1]->Setup (ns3UdpSocket3, sinkAddress2, sender+1, recv, packetSize, numPackets, mode,
               DataRate ("1Mbps"));
  dtnContainer.Get (1)->AddApplication (app[1]);
  app[1]->SetStartTime (Seconds (0.));
  app[1]->SetStopTime (Seconds (1000.));
  */

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
  Time timer = Seconds (50);
  //Ptr<OutputStreamWrapper> streamer = Create<OutputStreamWrapper> (&std::cout);
  Ptr<OutputStreamWrapper> streamer = Create<OutputStreamWrapper> ("tinko.txt", std::ios::out);
  olsrh.PrintRoutingTableAt (timer, olsrContainer.Get (0), streamer);

  Simulator::Schedule (Seconds (51), &fileopentest, "tinko.txt");
  Simulator::Schedule (Seconds (52), &kara);
  olsrh.PrintRoutingTableAt (Seconds (53), olsrContainer.Get (0), streamer);
  Simulator::Schedule (Seconds (53), &fileopentest, "tinko.txt");

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

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

/*
void
olsr_or_dtn (int sender, int receiver[])
{
  int recv = 0;
  int mode = 0;

  std::random_device rd{};

  while (1)
    {
      recv = rd () % numberOfueNodes + 1;
      if (recv != sender)
        {
          break;
        }
    }

  recv = 4;

  recv = recv - count;
  std::cout << "recv is " << recv << std::endl;

  for (int i = 0; i < R; i++)
    {
      std::cout << "receiver[" << i << "] (olsr_to_dtn) is " << receiver[i] << std::endl;
      if (receiver[i] <= recv)
        {
          if (recv == receiver[i])
              std::cout << "send by olsr" << std::endl;
              mode = 0;
              Address sinkAddress1 (InetSocketAddress (interfaces.GetAddress (recv - 1),
                                                       sinkPort)); // interface of n24

              std::cout << "(olst_or_dtn) count is " << count << std::endl;
              Ptr<Socket> ns3UdpSocket1 = Socket::CreateSocket (
                  olsrContainer.Get (0), UdpSocketFactory::GetTypeId ()); //source at n0
              app[count]->Setup (ns3UdpSocket1, sinkAddress1, sender, recv, packetSize, numPackets,
                                 mode, DataRate ("1Mbps"));
              olsrContainer.Get (0)->AddApplication (app[count]);
              app[count]->SetStartTime (Seconds (0.));
              app[count]->SetStopTime (Seconds (1000.));
              count++;
              break;
            }
        }

      if (receiver[i] > recv || receiver[i] == 0)
        {
          std::cout << "send by dtn" << std::endl;
          mode = 1;
          Address sinkAddress2 (InetSocketAddress (interfaces2.GetAddress (recv - 1),
                                                   sinkPort)); // interface of n24
          PacketSinkHelper packetSinkHelper2 ("ns3::UdpSocketFactory",
                                              InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
          ApplicationContainer sinkApps2 =
              packetSinkHelper2.Install (dtnContainer.Get (recv - 1)); //n2 as sink
          sinkApps2.Start (Seconds (0.));
          sinkApps2.Stop (Seconds (1000.));

          Ptr<Socket> ns3UdpSocket2 = Socket::CreateSocket (
              dtnContainer.Get (sender - 1), UdpSocketFactory::GetTypeId ()); //source at n0

          // Create UDP application at n0
          Ptr<MyApp> app2 = CreateObject<MyApp> ();
          app2->Setup (ns3UdpSocket2, sinkAddress2, sender, recv, packetSize, numPackets, mode,
                       DataRate ("1Mbps"));
          dtnContainer.Get (0)->AddApplication (app2);
          app2->SetStartTime (Seconds (0.));
          app2->SetStopTime (Seconds (1000.));
          break;
        }
    }
}
*/