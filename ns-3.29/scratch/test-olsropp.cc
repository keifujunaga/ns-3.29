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


// NetAnim
#include "ns3/netanim-module.h"

// Others
#include <boost/algorithm/string.hpp>
#include <vector>
#include <random>

#define R 100

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");


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
  NS_LOG_UNCOND("(MacRxDrop) packet uid is " << packet->GetUid());
}  

void 
MacTxDrop (std::string context, Ptr<const Packet> packet)
{
  NS_LOG_UNCOND("((MacTxDrop) packet uid is " << packet->GetUid());
}

/// WifiPhy

void
PhyTxDrop (std::string context, Ptr<const Packet> packet)
{
  NS_LOG_UNCOND("(PhyTxDrop) packet uid is " << packet->GetUid());
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
  
  NS_LOG_UNCOND("(PhyRxDrop) packet uid is " << packet->GetUid() << " packet size is " << packet->GetSize() << " node is " << kym);
  if (packet->GetSize() == 2112 && kym == 2){
    packetdropcounter += 2048;
  }
  NS_LOG_UNCOND("packetdropcounter is " << packetdropcounter);
}

/// Ipv4L3Protocol
void Drop (std::string context, const Ipv4Header & header, Ptr<const Packet> packet, Ipv4L3Protocol::DropReason reason, Ptr<Ipv4> ipv4, uint32_t interface)
{
  NS_LOG_UNCOND("tionko");  
}

///WifiPhyStateHelper
void RxError(std::string context, Ptr<const Packet> packet, double nanikore)
{
  NS_LOG_UNCOND("(RxError) packet uid is " << packet->GetUid());
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

  NS_LOG_UNCOND ("(ReceivedPacket) packet uid is " << packet->GetUid() << " packet size is " << packet->GetSize() << " node is " << kym);
}

/// MonitorSnifferRx
void 
MonitorSnifferRx(std::string context, Ptr<const Packet> packet, uint16_t channelFreqMhz, WifiTxVector txVector, MpduInfo aMpdu, SignalNoiseDbm signalNoise)
{
  NS_LOG_UNCOND("signal power is " << signalNoise.signal << ", noise power is " << signalNoise.noise);
}

void toString(std::ostream& out, std::string context)
{
  std::stringstream ss;
  ss << out.rdbuf();
  context = ss.str(); 
  std::cout << "context is " << context << std::endl;
}

int sender;
int receiver[R];

void fileopentest(std::string filename)
{
    std::ifstream ifs(filename);
    std::string str;
    std::string delim ("		."); 
    std::string test (": ,");
    std::list<std::string> list_string;
    std::string temp;
    int count = 0;
    int count2 = 0;

    if (ifs.fail())
    {
      std::cerr << "Failed to open file." << std::endl;
    }
    while (getline(ifs, str))
    {
      if (count==0)
      {
        boost::split(list_string, str, boost::is_any_of(test));
        auto itr = list_string.begin();
        itr++; itr++;
        std::cout << "itr is " << *itr << std::endl;
        sender = std::stoi(*itr);
        sender++;
        count++;
      }
      boost::split(list_string, str, boost::is_any_of(delim));
      auto itr = list_string.begin();
      if (*itr == "192" || *itr == "127")
      {
        if (*itr == "127") 
        {
          break;
        }
        ++itr; ++itr; ++itr;
        std::cout << "itr is " << *itr << std::endl;
        receiver[count2] = std::stoi(*itr);
        count2++;
      }
    }
}

void check_sendrecvlist()
{
  std::cout << "sender is " << sender << std::endl;
  for (int i = 0; i < R; i++)
  {
    if (receiver[i] == 0) break;
    std::cout << "receiver is " << receiver[i] << std::endl;
  }
}

NodeContainer olsrContainer;
NodeContainer dtnContainer;
NetDeviceContainer devices;
NetDeviceContainer devices2;
Ipv4InterfaceContainer interfaces;
Ipv4InterfaceContainer interfaces2;

// Parameter
std::string rate = "20.48kbps"; // 1.024kbps
uint32_t packetSize = 2048; // 2048
double appTotalTime = 125; // simulator stop time
double appDataStart = 0.0;
double appDataEnd = 125;
int numberOfueNodes = 4;

void olsr_or_dtn()
{
  int recv;
  std::random_device rd{};

  while (1)
  {
    recv = rd() % numberOfueNodes + 1;
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
       OnOffHelper onoff ("ns3::UdpSocketFactory", 
                      Address (InetSocketAddress (interfaces.GetAddress (recv-1), 80))); 
       onoff.SetConstantRate (DataRate (rate));
       onoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
       onoff.SetAttribute ("MaxBytes", UintegerValue(packetSize));
       ApplicationContainer apps = onoff.Install (olsrContainer.Get (sender-1));
       break;
      }
    } 
    
    if (receiver[i] > recv || receiver[i] == 0)
    {
      std::cout << "send by dtn" << std::endl;
        OnOffHelper dtnonoff ("ns3::UdpSocketFactory", 
                      Address (InetSocketAddress (interfaces2.GetAddress (recv-1), 80))); 
        dtnonoff.SetConstantRate (DataRate (rate));
        dtnonoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
        dtnonoff.SetAttribute ("MaxBytes", UintegerValue(packetSize));
        ApplicationContainer dtn_apps = dtnonoff.Install (dtnContainer.Get (sender-1));
        break;
    }
  }
}

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  // Time::SetResolution (Time::NS);
  LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);



  olsrContainer.Create(numberOfueNodes);
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
  mobility.Install(olsrContainer);
  mobility.Install(dtnContainer);
  
  /*
   *       Physical and link Layer Setup
   */


  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("Frequency", UintegerValue (2400)); //2400
  wifiPhy.Set ("TxPowerEnd", DoubleValue (30));
  wifiPhy.Set ("TxPowerStart", DoubleValue (30));
  wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue(-80));
  //wifiPhy.Set ("CcaModelThreshold", DoubleValue(-90));

  YansWifiChannelHelper wifiChannel;

  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel");

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
  OlsrHelper olsr;
  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

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
  internet2.Install(dtnContainer);
  Ipv4AddressHelper ipv4_2;  
  ipv4_2.SetBase ("192.168.2.0", "255.255.255.0");
  interfaces2 = ipv4_2.Assign (devices2);

/*
 *  setup sendeapp sinkr and receiver
 */

  PacketSinkHelper sink ("ns3::UdpSocketFactory", 
                        InetSocketAddress (Ipv4Address::GetAny (), 80));


  /*
   * setup app sink
   */
   
  ApplicationContainer olsr_app_sink = sink.Install (olsrContainer);
  olsr_app_sink.Start (Seconds (0.0));
  olsr_app_sink.Stop (Seconds (appTotalTime));

  ApplicationContainer dtn_app_sink = sink.Install(dtnContainer);
  dtn_app_sink.Start (Seconds (0.0));
  dtn_app_sink.Stop (Seconds (appTotalTime));
  
  /*
  for (int i=1; i < numberOfueNodes; i++)
  {
    OnOffHelper onoff ("ns3::UdpSocketFactory", 
                      Address (InetSocketAddress (interfaces.GetAddress (i), 80))); 
    onoff.SetConstantRate (DataRate (rate));
    onoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
    onoff.SetAttribute ("MaxBytes", UintegerValue(packetSize));
    ApplicationContainer apps = onoff.Install (olsrContainer.Get (0));
    apps.Start (Seconds (appDataStart+50));
    apps.Stop (Seconds (appTotalTime));
  }*/


  AsciiTraceHelper ascii;
  wifiPhy.EnableAsciiAll(ascii.CreateFileStream("test-tracesource.tr"));
  wifiPhy.EnablePcapAll("test-tracesource");

  Simulator::Stop (Seconds (appTotalTime));

  AnimationInterface anim ("test-olsropp-animation.xml"); 
  anim.EnablePacketMetadata(true);
  anim.EnableIpv4RouteTracking("tinkoko.xml", Seconds(0), Seconds(appDataEnd), Seconds(0.25));

  /*
   *  tracing olsr routingtable
   */
  std::string context;
  Time timer = Seconds(50);
  //Ptr<OutputStreamWrapper> streamer = Create<OutputStreamWrapper> (&std::cout);
  Ptr<OutputStreamWrapper> streamer = Create<OutputStreamWrapper> ("tinko.txt", std::ios::out);
  olsr.PrintRoutingTableAt(timer, olsrContainer.Get(0), streamer);

  Simulator::Schedule (Seconds(51), &fileopentest, "tinko.txt");
  Simulator::Schedule (Seconds(51), &check_sendrecvlist);
  Simulator::Schedule (Seconds(51), &olsr_or_dtn);


  //list.PrintRoutingTableAt(timer, olsrContainer.Get(1), streamer);




  // Mandatory  
  //Config::Connect ("NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRxDrop", MakeCallback (&MacRxDrop));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTxDrop", MakeCallback (&MacTxDrop));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback (&PhyTxDrop));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback (&PhyRxDrop));
  //Config::Connect ("NodeList/*/$ns3::Ipv4L3Protocol/Drop", MakeCallback(&Drop));
  Config::Connect ("NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",MakeCallback (&ReceivedPacket));
  //Config::Connect("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/State/RxError", MakeCallback(&RxError));
 //Config::Connect("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx", MakeCallback(&MonitorSnifferRx));

  
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
