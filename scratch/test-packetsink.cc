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
#include <ns3/buildings-module.h>


// NetAnim
#include "ns3/netanim-module.h"

// Others
#include <boost/algorithm/string.hpp>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

// 
void
CourseChangeaa (std::string context, Ptr<const MobilityModel> model)
{
  std::stringstream ss;
  ss << "Mobility.log";
  static std::fstream f (ss.str ().c_str (), std::ios::out);
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  Vector position = model->GetPosition ();
  f << "SIMTIME = " << Simulator::Now ().GetSeconds () << " NODE = " << kym << " X = " << position.x
    << " Y = " << position.y << std::endl;
}

void
phyMonitorSniffRx (std::string context, Ptr<const Packet> packet, uint16_t i, WifiTxVector txvec,
                   MpduInfo mpdu, SignalNoiseDbm sdb)
{
  std::stringstream ss;
  ss << "MonitorSnifferRx.log";
  static std::fstream f (ss.str ().c_str (), std::ios::out);

  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  f << "SIMTIME = " << Simulator::Now ().GetSeconds () << " NODE = " << kym
    << " PKT_UID = " << packet->GetUid () << " PKT_SIZE = " << packet->GetSize ()
    << " SIGNAL_POWER = " << sdb.signal << std::endl;

  if(packet->GetSize () == 2112){
    f << Simulator::Now ().GetSeconds () << ", " << Simulator::Now ().GetSeconds () * 8 << ", " << sdb.signal << std::endl;
  }

  /*
  NS_LOG_UNCOND(context << "SIGNAL_POWER = " <<  sdb.signal << "    NOISE_POWER = " << sdb.noise << "   AMPDU_REFNUM = " << mqdu.mpduRefNumber  << "  PKT_SIZE = " << packet->GetSize() << "  " << "   = MonitorSnifferRx");
  */

  /*
  f << "SIMTIME = " << Simulator::Now().GetSeconds() <<" NODE = " << kym << " SIGNAL_POWER = " <<  sdb.signal << " NOISE_POWER = " << sdb.noise << " AMPDU_REFNUM = " << mpdu.mpduRefNumber  << " PKT_UID = " << packet->GetUid() <<  " PKT_SIZE = " << packet->GetSize() << " NTx = " << txvec.GetGuardInterval() << std::endl;
  */
}

void
phyMonitorSniffTx (std::string context, Ptr<const Packet> packet, uint16_t channelFreqMhz,
                   WifiTxVector txVector, MpduInfo aMpdu)
{
  std::stringstream ss;
  ss << "MonitorSnifferTx.log";
  static std::fstream f (ss.str ().c_str (), std::ios::out);

  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  f << "SIMTIME = " << Simulator::Now ().GetSeconds () << " NODE = " << kym
    << " PKT_UID = " << packet->GetUid () << " PKT_SIZE = " << packet->GetSize () << std::endl;
}

void ReceivedPacket (std::string context, Ptr<const Packet> packet, const Address &addr)
{
  std::cout << "addr is " << addr << std::endl;
}

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  // Time::SetResolution (Time::NS);
  LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);

  // Parameter
  std::string rate = "20.48kbps"; // 1.024kbps
  uint32_t packetSize = 2048; // 2048
  double appTotalTime = 125; // simulator stop time
  double appDataStart = 0.0;
  double appDataEnd = 125;
  int numberOfueNodes = 2;

  NodeContainer nodeContainer;
  nodeContainer.Create(2);

  /*
   *      Mobility model Setup
   */

  MobilityHelper mobility;

  Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator> ();
  initialAlloc->Add (Vector (0, 0, 200));
  initialAlloc->Add (Vector (0, 0, 1.5));
  mobility.SetPositionAllocator (initialAlloc);
 
  mobility.SetMobilityModel ("ns3::WaypointMobilityModel");
  mobility.Install(nodeContainer);
  for (int u = 0; u < numberOfueNodes; u++){
    Ptr<WaypointMobilityModel> waypoint =
    DynamicCast<WaypointMobilityModel> (nodeContainer.Get (u)->GetObject<MobilityModel> ());
    if (u == 0){
      waypoint->AddWaypoint (Waypoint (Seconds (appDataStart), Vector (0, 0, 200)));
      waypoint->AddWaypoint (Waypoint (Seconds (appTotalTime), Vector (-500, 0, 200)));
    }
    if (u == 1){
      waypoint->AddWaypoint (Waypoint (Seconds (appDataStart), Vector (0, 0, 1.5)));
      waypoint->AddWaypoint (Waypoint (Seconds (appTotalTime), Vector (500, 0, 1.5)));
    }
  }

  BuildingsHelper::Install (nodeContainer);
  BuildingsHelper::MakeMobilityModelConsistent ();

  NetDeviceContainer devices;

  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("Frequency", UintegerValue (2400)); //2400
  wifiPhy.Set ("TxPowerEnd", DoubleValue (30));
  wifiPhy.Set ("TxPowerStart", DoubleValue (30));
  // wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue(-90.0));
  // wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); ///
  //YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  YansWifiChannelHelper wifiChannel;

  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // wifiChannel.AddPropagationLoss ("ns3::HybridBuildingsPropagationLossModel");   
  // wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel");
  // wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");
  // wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
  // wifiChannel.AddPropagationLoss("ns3::OkumuraHataPropagationLossModel");
  wifiChannel.AddPropagationLoss("ns3::ExtendedHataModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a); ///

  //wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
  //                              StringValue ("OfdmRate6Mbps"), ///
  //                              "RtsCtsThreshold", UintegerValue (0));
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


  // Sink or server setup
  PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
  ApplicationContainer apps_sink = sink.Install (nodeContainer.Get (0));
  apps_sink.Start (Seconds (0.0));
  apps_sink.Stop (Seconds (appTotalTime));

  // Client setup
  OnOffHelper onoff1 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (0), 80)));
  onoff1.SetConstantRate (DataRate (rate));
  onoff1.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps1 = onoff1.Install (nodeContainer.Get (1));
  apps1.Start (Seconds (appDataStart));
  apps1.Stop (Seconds (appDataEnd));

  Simulator::Stop (Seconds (appTotalTime));

  AnimationInterface anim ("myfitst-animation.xml"); // Mandatory

  std::ostringstream oss;
  std::ostringstream oss2;
  std::ostringstream oss3;
  std::ostringstream oss4;

  oss << "/NodeList/*/$ns3::MobilityModel/CourseChange";
  oss2 << "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx";
  oss3 << "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferTx";
  oss4 << "NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx";

  Config::Connect (oss.str (), MakeCallback (&CourseChangeaa));
  Config::Connect (oss2.str (), MakeCallback (&phyMonitorSniffRx));
  Config::Connect (oss3.str (), MakeCallback (&phyMonitorSniffTx));
  Config::Connect (oss4.str (), MakeCallback (&ReceivedPacket));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
