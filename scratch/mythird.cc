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
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"

// NetAnim
#include "ns3/netanim-module.h"

// Others
#include <boost/algorithm/string.hpp>
#include <vector>

// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

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

int
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 3;
  uint32_t nWifi = 3;
  bool tracing = false;

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc, argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box"
                << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode = p2pNodes.Get (0);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator", "MinX", DoubleValue (0.0), "MinY",
                                 DoubleValue (0.0), "DeltaX", DoubleValue (5.0), "DeltaY",
                                 DoubleValue (10.0), "GridWidth", UintegerValue (3), "LayoutType",
                                 StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds",
                             RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodes);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  InternetStackHelper stack;
  stack.Install (csmaNodes);
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (wifiStaNodes.Get (nWifi - 1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));

  if (tracing == true)
    {
      pointToPoint.EnablePcapAll ("third");
      phy.EnablePcap ("third", apDevices.Get (0));
      csma.EnablePcap ("third", csmaDevices.Get (0), true);
    }

  std::ostringstream oss;
  std::ostringstream oss2;
  std::ostringstream oss3;

  oss << "/NodeList/*/$ns3::MobilityModel/CourseChange";
  oss2 << "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx";
  oss3 << "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferTx";

  // oss <<
  // "/NodeList/" << wifiStaNodes.Get (nWifi - 1)->GetId () <<
  // "/$ns3::MobilityModel/CourseChange";
  //oss2 <<
  //"/NodeList/" << wifiStaNodes.Get (nWifi - 1)->GetId () <<
  //"/Devicelist/" << "*" << "/$ns3::WifiPhy/MonitorSnifferRx";

  Config::Connect (oss.str (), MakeCallback (&CourseChangeaa));
  Config::Connect (oss2.str (), MakeCallback (&phyMonitorSniffRx));
  Config::Connect (oss3.str (), MakeCallback (&phyMonitorSniffTx));

  AnimationInterface anim ("mythird-animation.xml"); // Mandatory

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
