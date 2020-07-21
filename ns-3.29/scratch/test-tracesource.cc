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
#include <ns3/buildings-module.h>


// NetAnim
#include "ns3/netanim-module.h"

// Others
#include <boost/algorithm/string.hpp>
#include <vector>

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

void fileopentest(std::string filename)
{
    std::ifstream ifs(filename);
    std::string str;
    std::string delim ("		."); 
    std::list<std::string> list_string;
    std::string temp;

    if (ifs.fail())
    {
      std::cerr << "Failed to open file." << std::endl;
    }
    while (getline(ifs, str))
    {
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
      }
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

  // Parameter
  std::string rate = "20.48kbps"; // 1.024kbps
  uint32_t packetSize = 2048; // 2048
  double appTotalTime = 125; // simulator stop time
  double appDataStart = 0.0;
  double appDataEnd = 125;
  int numberOfueNodes = 4;

  NodeContainer nodeContainer;
  nodeContainer.Create(numberOfueNodes);

  FlowMonitorHelper flowhelper;
  flowhelper.InstallAll();

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
  mobility.Install(nodeContainer);
  
  NetDeviceContainer devices;

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
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a); ///

  devices = wifi.Install (wifiPhy, wifiMac, nodeContainer);

  Ipv4StaticRoutingHelper staticRouting;
  OlsrHelper olsr;
  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (list);
  internet.Install (nodeContainer);
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

   PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));

  /// node1
  ApplicationContainer apps_sink = sink.Install (nodeContainer);
  apps_sink.Start (Seconds (0.0));
  apps_sink.Stop (Seconds (appTotalTime));
  
  for (int i=1; i < numberOfueNodes; i++)
  {
    OnOffHelper onoff ("ns3::UdpSocketFactory", 
                      Address (InetSocketAddress (interfaces.GetAddress (i), 80))); 
    onoff.SetConstantRate (DataRate (rate));
    onoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
    ApplicationContainer apps = onoff.Install (nodeContainer.Get (0));
    apps.Start (Seconds (appDataStart));
    apps.Stop (Seconds (appTotalTime));
  }

  AsciiTraceHelper ascii;
  wifiPhy.EnableAsciiAll(ascii.CreateFileStream("test-tracesource.tr"));
  wifiPhy.EnablePcapAll("test-tracesource");

  Simulator::Stop (Seconds (appTotalTime));

  flowhelper.SerializeToXmlFile("test-tracesource.xml", false, false);
  AnimationInterface anim ("myfitst-animation.xml"); 
  anim.EnableIpv4RouteTracking("tinkoko.xml", Seconds(0), Seconds(appDataEnd), Seconds(0.25));

  std::string context;
  Time timer = Seconds(50);
  //Ptr<OutputStreamWrapper> streamer = Create<OutputStreamWrapper> (&std::cout);
  Ptr<OutputStreamWrapper> streamer = Create<OutputStreamWrapper> ("tinko.txt", std::ios::out);
  olsr.PrintRoutingTableAt(timer, nodeContainer.Get(0), streamer);
  Simulator::Schedule (Seconds(51), &fileopentest, "tinko.txt");
  

  if (Simulator::Now().GetSeconds() >= Seconds(51))
  {
    std::cout << "asasahi " << std::endl;
    /*
    *streamer->GetStream() << Simulator::Now().GetSeconds() << std::endl;
    std::cout << "asasa " << std::endl;
    context = toString(*streamer->GetStream());
    std::cout << "asasahi " << std::endl;
    */
    
    //std::cout << "context is " << context << std::endl;

    /*
    std::ifstream ifs("tinko.txt");
    std::string str;

    if (ifs.fail())
    {
      std::cerr << "Failed to open file." << std::endl;
      return -1;
    }
    while (getline(ifs, str))
    {
      std::cout << "#" << str << std::endl;
    }
    */
  }



  //list.PrintRoutingTableAt(timer, nodeContainer.Get(1), streamer);




  // Mandatory  
  //Config::Connect ("NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRxDrop", MakeCallback (&MacRxDrop));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTxDrop", MakeCallback (&MacTxDrop));
  //Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback (&PhyTxDrop));
  Config::Connect ("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback (&PhyRxDrop)); /// sousinkanryou sitemo drop
  //Config::Connect ("NodeList/*/$ns3::Ipv4L3Protocol/Drop", MakeCallback(&Drop));
  //Config::Connect ("NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",MakeCallback (&ReceivedPacket));
  //Config::Connect("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/State/RxError", MakeCallback(&RxError));
  //Config::Connect("NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx", MakeCallback(&MonitorSnifferRx));

  
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
