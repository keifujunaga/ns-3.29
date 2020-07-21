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

//int recv;
//int sender;
//int receiver[R];
//int uid;
//bool olsrdtn[100000][30];

// prototype sengen
void olsr_or_dtn (int sender);
void dtn_to_olsr (int sender, int recv, int uid);
void olsr_testApp (int recv, int sender, int mode, int uid);
void dtn_testApp (int recv, int sender, int mode, int uid);

// typedef
/*
typedef std::map<std::pair<Ipv4Address, Ipv4Address>, std::vector<Ipv4Address>> Tables;
typedef std::map<std::pair<Ipv4Address, Ipv4Address>, Ipv4Address> NextHops;
Tables tables;
NextHops next_hops;
*/

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
  IPv4TestTag tag;
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  /*
  std::cout << "(MyApp) "
            << "packet uid is " << packet->GetUid () << std::endl;
  std::cout << "(MyApp) "
            << "packet recvnode is " << m_receiver << std::endl;
  std::cout << "(MyApp) "
            << "mode is " << m_mode << std::endl;
  std::cout << "(MyApp) "
            << "uid is " << m_uid << std::endl;
  std::cout << "(MyApp) "
            << "sender is " << m_sender << std::endl;
  std::cout << "(MyApp) "
            << "receiver is " << m_receiver << std::endl;
  */

  if (m_uid == -1)
  {
    tag.SetToken (packet->GetUid () * 10000 + m_receiver * 10 + m_mode);
    packet->AddPacketTag (tag);
    mem.insert (std::make_pair(packet->GetUid()*10+m_sender, true));
    NS_LOG_INFO("(MyApp) send packet (uid is " << packet->GetUid() << ") by sender "  << m_sender);
    m_socket->Send (packet);
  }
  else
  { 
    tag.SetToken (m_uid * 10000 + m_receiver * 10 + m_mode);
    packet->AddPacketTag (tag);
    //std::cout << "mem[" << m_uid*10+m_sender << "] is " << mem[m_uid*10+m_sender] << std::endl;
    if (mem[m_uid*10+m_sender] == false)  
    {
      NS_LOG_INFO("(MyApp) send packet (uid is " <<packet->GetUid() << ") by sender "  << m_sender);
      m_socket->Send (packet);
    }
    //std::cout << "insert m_uid, m_sender" << m_uid << ", " << m_sender << std::endl;
    mem[m_uid*10+m_sender] = true;
    //mem.insert (std::make_pair(m_uid*10+m_sender, true));
    //std::cout << "mem[" << m_uid*10+m_sender << "] is " << mem[m_uid*10+m_sender] << std::endl;
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
  //Ptr<std::vector<Ipv4Address>> addresses;
  if (packet->GetSize () >= 2048)
    {
      //epidemic.Drop(dtnContainer);
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
              /*
              std::cout << "(MonitorSnifferRx) "
                        << "Time:" << Simulator::Now().GetSeconds() << ", "
                        << "uid:" << uid << ", "
                        << "recv:" << kym << ", "
                        << "mode:" << mode << std::endl;
              std::cout << "(MonitorSnifferRx) node " << kym << " is using dtn." 
                        << "Then execute dtn_to_olsr" << std::endl;
              */
              int time = Simulator::Now ().GetSeconds ();
              int endtime = (int) appDataEnd - time;
              for (int i = 0; i < endtime; i++)
                {
                  /*
                  std::cout << "(MonitorSnifferRx) " 
                            << "Time:" << i << ", "
                            << "Schedule dtn_to_olsr" << std::endl; 
                  */
                  Simulator::Schedule (Seconds (i), &dtn_to_olsr, kym, recv, uid);
                }
            }
        }
    }
}

void 
TraceSendPacket (std::string context, Ptr<const Packet> packet, Ipv4Address dest)
{
  NS_LOG_INFO ("(TraceSendPacket) " << "Time: " << Simulator::Now().GetSeconds());
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo
  NS_LOG_INFO ("(TraceSendPacket) " << "Node: " << kym);
  NS_LOG_INFO ("(TraceSendPacket) " << "DestAddress: " << dest);
  
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
  NS_LOG_INFO ("(TraceSendPacket) " << "recvnodenum : " << recvnodenum );
  Ptr<Node> node = olsrContainer.Get (recvnodenum);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ipv4Address recvaddr = ipv4->GetAddress (1, 0).GetLocal ();
  NS_LOG_INFO ("(TraceSendPacket) " << "recvaddr : " << recvaddr);

  bool judge;
  Ipv4Address table_addr[10];
  olsrh.aaa(NodeList::GetNode(kym-numberOfueNodes), table_addr);
  for (int i = 0; i < 10; i++)
  {
    NS_LOG_INFO ("(TraceSendPacket) table_addr[" << i << "] is " << table_addr[i] );
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
  NS_LOG_INFO ("(TraceSendPacket) " << "judge: " << judge);
}

void TraceupdateDeliveryPredFor(std::string context, Ipv4Address mainAddress, Ipv4Address addr, double newValue)
{
  mainmap[mainAddress][addr] = newValue;
  NS_LOG_WARN("main.cc : P(" << mainAddress << "," << addr << ") is " << mainmap[mainAddress][addr]);
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

  NS_LOG_WARN ("TraceSendPacketFromQueue");

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
  NS_LOG_INFO ("(olsr_or_dtn) " << "recvaddr:" << recvaddr);

  for (int i = 0; i < 10; i++)
    {
      NS_LOG_INFO ("(olsr_or_dtn) table_addr[" << i << "] is " << table_addr[i]);
      if (recvaddr == table_addr[i])
        {
          NS_LOG_INFO ("(olsr_or_dtn) send by olsr");
          mode = 0;
          olsr_testApp (recv, sender, mode, uid);
          break;
        }
      if (table_addr[i] == Ipv4Address ("102.102.102.102"))
        {
          NS_LOG_INFO ("(olsr_or_dtn) send by dtn");
          mode = 1;
          dtn_testApp (recv, sender, mode, uid);
          break;
        }
    }
}

void
dtn_to_olsr (int sender, int recv, int uid)
{
  /*
  std::cout << "(dtn_to_olsr) before "
                    << "sender Node : " << sender
                    << " receiver Node : " << recv
                    << " sends by olsr at " 
                    <<  Simulator::Now().GetSeconds() 
                    << std::endl;
  */
  Ptr<Node> node = olsrContainer.Get (recv);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ipv4Address recvaddr = ipv4->GetAddress (1, 0).GetLocal ();

  Ipv4Address table_addr[10];
  olsrh.aaa (NodeList::GetNode (sender), table_addr);

  // rinsetu node no test
  /*
  std::cout << "(dtn_to_olsr) number of neighbors of a device: " 
            << NodeList::GetNode(sender)->GetNDevices()
            << std::endl;
  */

  /*
  for (uint32_t i = 0 ; i < NodeList::GetNode(sender)->GetNDevices(); ++i)
  {
    Ipv4InterfaceAddress iface = ipv4->GetAddress (ipv4->GetInterfaceForDevice(node->GetDevice(i)), 0);
    std::cout << "iface is " << iface << std::endl;
  }

  for (int i = 0; i < 10; i++)
  {
    std::cout << "(dtn_to_olsr) "
              << "Time " << Simulator::Now().GetSeconds() << ", "
              << "table_addr[" << i << "] is " << table_addr[i] << std::endl;
  }
  */

  for (int i = 0; i < 10; i++)
    {
      if (recvaddr == table_addr[i] && mem[uid*10+sender] == false)
        {
          NS_LOG_INFO ("(dtn_to_olsr) "
                    << "Node : " << sender
                    << " sends by olsr at " 
                    <<  Simulator::Now().GetSeconds());
          std::cout << "(dtn_to_olsr) "
                    << "Node : " << sender
                    << " sends by olsr at " 
                    <<  Simulator::Now().GetSeconds() << std::endl;
                   
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

/*
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
kara ()
{
  system ("echo -n > tinko.txt");
}
*/