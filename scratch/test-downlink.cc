#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/epidemic-helper.h"
#include <ns3/buildings-module.h>
#include "ns3/netanim-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/aodv-module.h"

#include <iostream>
#include <ctime>
#include <cstdlib>

// Others
#include <boost/algorithm/string.hpp>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("EpidemicExample");

void
CourseChange (std::string context, Ptr<const MobilityModel> model)
{
  Vector position = model->GetPosition ();
  NS_LOG_UNCOND ("Simulation time " << Simulator::Now ().GetSeconds ());
  NS_LOG_UNCOND (context << " x = " << position.x << ", y = " << position.y
                         << ", z = " << position.z);
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

  /*
  f << "SIMTIME = " << Simulator::Now ().GetSeconds () << " NODE = " << kym
    << " PKT_UID = " << packet->GetUid () << " PKT_SIZE = " << packet->GetSize ()
    << " SIGNAL_POWER = " << sdb.signal << std::endl;
  */
   f << kym << ", " << packet->GetUid() << ", " << sdb.signal << std::endl;

  /*
  NS_LOG_UNCOND(context << "SIGNAL_POWER = " <<  sdb.signal << "    NOISE_POWER = " << sdb.noise << "   AMPDU_REFNUM = " << mqdu.mpduRefNumber  << "  PKT_SIZE = " << packet->GetSize() << "  " << "   = MonitorSnifferRx");
  */

  /*
  f << "SIMTIME = " << Simulator::Now().GetSeconds() <<" NODE = " << kym << " SIGNAL_POWER = " <<  sdb.signal << " NOISE_POWER = " << sdb.noise << " AMPDU_REFNUM = " << mpdu.mpduRefNumber  << " PKT_UID = " << packet->GetUid() <<  " PKT_SIZE = " << packet->GetSize() << " NTx = " << txvec.GetGuardInterval() << std::endl;
  */
}

int
main (int argc, char *argv[])
{
  // General parameters
  double nodeSpeed = 1.23; //speed no henkou
  bool app_logging = true;
  //std::string rate = "0.128kbps";
  //std::string rate = "0.512kbps";
  //std::string rate = "2.048kbps";
  std::string rate = "4.096kbps";
  //uint32_t packetSize = 64;
  uint32_t packetSize = 2048;
  double appTotalTime = 700.0; // simulator stop time
  double appDataStart = 100.0;
  double appDataEnd = 600.0;
  uint32_t randnum = 1;

  // Epidemic parameters
  uint32_t epidemicHopCount = 1000;
  uint32_t epidemicQueueLength = 1000;
  Time epidemicQueueEntryExpireTime = Seconds (700);
  Time epidemicBeaconInterval = Seconds (1);

  /*
  Allow users to override the default parameters and set it to
  new ones from CommandLine.
  */
  CommandLine cmd;
  cmd.AddValue ("rate", "CBR traffic rate(in kbps)", rate);
  cmd.AddValue ("packetSize", "The packet size", packetSize);
  cmd.AddValue ("nodeSpeed", "Node speed in RandomWayPoint model", nodeSpeed);
  cmd.AddValue ("Hop Count", "number of hops before a packet is dropped", epidemicHopCount);
  cmd.AddValue ("QueueLength", "Specify queue Length", epidemicQueueLength);
  cmd.AddValue ("QueueEntryExpireTime", "Specify queue Entry Expire Time",
                epidemicQueueEntryExpireTime);
  cmd.AddValue ("BeaconInterval", "Specify beaconInterval", epidemicBeaconInterval);
  cmd.AddValue ("Randnum", "Specify initial Allocator ramdom seed", randnum);
  cmd.Parse (argc, argv);

  std::cout << "Node speed: " << nodeSpeed << " m/s" << std::endl;
  std::cout << "Packet size: " << packetSize << " b" << std::endl;
  std::cout << "Hop count: " << epidemicHopCount << std::endl;
  std::cout << "Queue length: " << epidemicQueueLength << " packets" << std::endl;
  std::cout << "Queue entry expire time: " << epidemicQueueEntryExpireTime.GetSeconds () << " s"
            << std::endl;
  std::cout << "Beacon interval: " << epidemicBeaconInterval.GetSeconds () << " s" << std::endl;

  /*
   *       Enabling OnOffApplication and PacketSink logging
   */
  if (app_logging)
    {
      LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
      LogComponentEnableAll (LOG_PREFIX_TIME);
      LogComponentEnableAll (LOG_PREFIX_NODE);
      LogComponentEnableAll (LOG_PREFIX_FUNC);
    }

  LogComponentEnable ("EpidemicExample", LOG_LEVEL_ALL);

  NodeContainer nodeContainer;
  NodeContainer uav;
  NodeContainer mobilenode;
  NetDeviceContainer devices;

  uav.Create (30);
  mobilenode.Create (12);
  nodeContainer.Add (uav);
  nodeContainer.Add (mobilenode);

  /*
   *       Mobility model Setup
   */
  MobilityHelper mobility;
  MobilityHelper mobility2;

  Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator> ();
  initialAlloc->Add (Vector (-132.374,-526.024,200));
  initialAlloc->Add (Vector (98.892,-507.877,200));
  initialAlloc->Add (Vector (330.156,-489.77,200));
  initialAlloc->Add (Vector (561.433,-471.672,200));
  initialAlloc->Add (Vector (792.715,-453.592,200));
  initialAlloc->Add (Vector (-263.759,-334.822,200));
  initialAlloc->Add (Vector (-32.4805,-316.644,200));
  initialAlloc->Add (Vector (198.789,-298.511,200));
  initialAlloc->Add (Vector (430.068,-280.414,200));
  initialAlloc->Add (Vector (661.364,-262.304,200));
  initialAlloc->Add (Vector (-163.901,-125.441,200));
  initialAlloc->Add (Vector (67.365,-107.266,200));
  initialAlloc->Add (Vector (298.635,-89.1538,200));
  initialAlloc->Add (Vector (529.908,-71.0557,200));
  initialAlloc->Add (Vector (761.172,-52.8765,200));
  initialAlloc->Add (Vector (-64.0944,83.9596,200));
  initialAlloc->Add (Vector (167.183,102.104,200));
  initialAlloc->Add (Vector (398.454,120.205,200));
  initialAlloc->Add (Vector (629.719,138.325,200));
  initialAlloc->Add (Vector (861.003,156.524,200));
  initialAlloc->Add (Vector (-195.536,275.167,200));
  initialAlloc->Add (Vector (35.7474,293.348,200));
  initialAlloc->Add (Vector (267.027,311.469,200));
  initialAlloc->Add (Vector (498.298,329.569,200));
  initialAlloc->Add (Vector (729.563,347.688,200));
  initialAlloc->Add (Vector (-95.6565,484.572,200));
  initialAlloc->Add (Vector (135.642,502.723,200));
  initialAlloc->Add (Vector (366.935,520.828,200));
  initialAlloc->Add (Vector (598.219,538.93,200));
  initialAlloc->Add (Vector (829.513,557.012,200));

  mobility.SetPositionAllocator (initialAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  // mobility.Install (uav);
  for (int i = 0; i < 30; i++)
    {
      mobility.Install (uav.Get (i));
    }

  mobility2.SetPositionAllocator ("ns3::GridPositionAllocator",
  "MinX", DoubleValue (36),
  "MinY", DoubleValue (-52),
  "DeltaX", DoubleValue (100),
  "DeltaY", DoubleValue (50),
  "Z", DoubleValue(1.5),
  "GridWidth", UintegerValue (4),
  "LayoutType", StringValue ("RowFirst")
  );

  mobility2.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
  "Bounds", RectangleValue (Rectangle (36, 729, -52, 347)));

  mobility2.Install(mobilenode);

  // BuildingsHelper::Install (nodeContainer);
  // BuildingsHelper::MakeMobilityModelConsistent ();
  // mobility.Install (nodeContainer);

  /*
   *       Physical and link Layers Setup
   */

  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("Frequency", UintegerValue (2400));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (30));
  wifiPhy.Set ("TxPowerStart", DoubleValue (30));
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); ///
  //YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  YansWifiChannelHelper wifiChannel;

  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::ExtendedHataModel");   
  // wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel");
  wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");

  wifiPhy.SetChannel (wifiChannel.Create ());

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a); ///

  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                StringValue ("OfdmRate6Mbps"), ///
                                "RtsCtsThreshold", UintegerValue (0));
  devices = wifi.Install (wifiPhy, wifiMac, nodeContainer);

  /*
   *       Epidemic Routing Setup
   */
  EpidemicHelper epidemic;
  epidemic.Set ("HopCount", UintegerValue (epidemicHopCount));
  epidemic.Set ("QueueLength", UintegerValue (epidemicQueueLength));
  epidemic.Set ("QueueEntryExpireTime", TimeValue (epidemicQueueEntryExpireTime));
  epidemic.Set ("BeaconInterval", TimeValue (epidemicBeaconInterval));

  Ipv4StaticRoutingHelper staticRouting;
  OlsrHelper olsr;
  // AodvHelper aodv;

  /*
   *       Internet Stack Setup
   */
  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

  InternetStackHelper internet;
  // internet.SetRoutingHelper (epidemic);
  internet.SetRoutingHelper (list);
  internet.Install (nodeContainer);
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  /*
   *         Application Setup
   */

  // Sink or server setup
  PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));

  /// 30
  ApplicationContainer apps_sink = sink.Install (nodeContainer.Get (30));
  apps_sink.Start (Seconds (0.0));
  apps_sink.Stop (Seconds (appTotalTime));

  OnOffHelper onoff1 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (30), 80)));
  onoff1.SetConstantRate (DataRate (rate));
  onoff1.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps1 = onoff1.Install (nodeContainer.Get (0));
  apps1.Start (Seconds (appDataStart));
  apps1.Stop (Seconds (appDataEnd));

  /// 31
  ApplicationContainer apps_sink2 = sink.Install (nodeContainer.Get (31));
  apps_sink2.Start (Seconds (0.0));
  apps_sink2.Stop (Seconds (appTotalTime));

  OnOffHelper onoff2 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (31), 80)));
  onoff2.SetConstantRate (DataRate (rate));
  onoff2.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps2 = onoff2.Install (nodeContainer.Get (0));
  apps2.Start (Seconds (appDataStart));
  apps2.Stop (Seconds (appDataEnd));

  /// 32
  ApplicationContainer apps_sink3 = sink.Install (nodeContainer.Get (32));
  apps_sink3.Start (Seconds (0.0));
  apps_sink3.Stop (Seconds (appTotalTime));

  OnOffHelper onoff3 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (32), 80)));
  onoff3.SetConstantRate (DataRate (rate));
  onoff3.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps3 = onoff3.Install (nodeContainer.Get (0));
  apps3.Start (Seconds (appDataStart));
  apps3.Stop (Seconds (appDataEnd));

  /// 33
  ApplicationContainer apps_sink4 = sink.Install (nodeContainer.Get (33));
  apps_sink4.Start (Seconds (0.0));
  apps_sink4.Stop (Seconds (appTotalTime));

  OnOffHelper onoff4 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (33), 80)));
  onoff4.SetConstantRate (DataRate (rate));
  onoff4.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps4 = onoff4.Install (nodeContainer.Get (0));
  apps4.Start (Seconds (appDataStart));
  apps4.Stop (Seconds (appDataEnd));

  /// 34
  ApplicationContainer apps_sink5 = sink.Install (nodeContainer.Get (34));
  apps_sink5.Start (Seconds (0.0));
  apps_sink5.Stop (Seconds (appTotalTime));

  OnOffHelper onoff5 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (34), 80)));
  onoff5.SetConstantRate (DataRate (rate));
  onoff5.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps5 = onoff5.Install (nodeContainer.Get (0));
  apps5.Start (Seconds (appDataStart));
  apps5.Stop (Seconds (appDataEnd));

  /// 35
  ApplicationContainer apps_sink6 = sink.Install (nodeContainer.Get (35));
  apps_sink6.Start (Seconds (0.0));
  apps_sink6.Stop (Seconds (appTotalTime));

  OnOffHelper onoff6 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (35), 80)));
  onoff6.SetConstantRate (DataRate (rate));
  onoff6.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps6 = onoff6.Install (nodeContainer.Get (0));
  apps6.Start (Seconds (appDataStart));
  apps6.Stop (Seconds (appDataEnd));

  /// 36
  ApplicationContainer apps_sink7 = sink.Install (nodeContainer.Get (36));
  apps_sink7.Start (Seconds (0.0));
  apps_sink7.Stop (Seconds (appTotalTime));

  OnOffHelper onoff7 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (36), 80)));
  onoff7.SetConstantRate (DataRate (rate));
  onoff7.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps7 = onoff7.Install (nodeContainer.Get (0));
  apps7.Start (Seconds (appDataStart));
  apps7.Stop (Seconds (appDataEnd));

  /// original 31
  ApplicationContainer apps_sink8 = sink.Install (nodeContainer.Get (37));
  apps_sink8.Start (Seconds (0.0));
  apps_sink8.Stop (Seconds (appTotalTime));

  OnOffHelper onoff8 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (37), 80)));
  onoff8.SetConstantRate (DataRate (rate));
  onoff8.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps8 = onoff8.Install (nodeContainer.Get (0));
  apps8.Start (Seconds (appDataStart));
  apps8.Stop (Seconds (appDataEnd));

  /// 32
  ApplicationContainer apps_sink9 = sink.Install (nodeContainer.Get (38));
  apps_sink9.Start (Seconds (0.0));
  apps_sink9.Stop (Seconds (appTotalTime));

  OnOffHelper onoff9 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (38), 80)));
  onoff9.SetConstantRate (DataRate (rate));
  onoff9.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps9 = onoff9.Install (nodeContainer.Get (0));
  apps9.Start (Seconds (appDataStart));
  apps9.Stop (Seconds (appDataEnd));

  /// 33
  ApplicationContainer apps_sink10 = sink.Install (nodeContainer.Get (39));
  apps_sink10.Start (Seconds (0.0));
  apps_sink10.Stop (Seconds (appTotalTime));

  OnOffHelper onoff10 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (39), 80)));
  onoff10.SetConstantRate (DataRate (rate));
  onoff10.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps10 = onoff10.Install (nodeContainer.Get (0));
  apps10.Start (Seconds (appDataStart));
  apps10.Stop (Seconds (appDataEnd));

  /// 34
  ApplicationContainer apps_sink11 = sink.Install (nodeContainer.Get (40));
  apps_sink11.Start (Seconds (0.0));
  apps_sink11.Stop (Seconds (appTotalTime));

  OnOffHelper onoff11 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (40), 80)));
  onoff11.SetConstantRate (DataRate (rate));
  onoff11.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps11 = onoff11.Install (nodeContainer.Get (0));
  apps11.Start (Seconds (appDataStart));
  apps11.Stop (Seconds (appDataEnd));

  /// 35
  ApplicationContainer apps_sink12 = sink.Install (nodeContainer.Get (41));
  apps_sink12.Start (Seconds (0.0));
  apps_sink12.Stop (Seconds (appTotalTime));

  OnOffHelper onoff12 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (41), 80)));
  onoff12.SetConstantRate (DataRate (rate));
  onoff12.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps12 = onoff12.Install (nodeContainer.Get (0));
  apps12.Start (Seconds (appDataStart));
  apps12.Stop (Seconds (appDataEnd));

  Simulator::Stop (Seconds (appTotalTime));

  AnimationInterface anim ("wireless-animation.xml"); // Mandatory

  std::ostringstream oss;
  std::ostringstream oss2;

  oss << "/NodeList/*/$ns3::MobilityModel/CourseChange";
  oss2 << "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx";

  // Config::Connect (oss.str (), MakeCallback (&CourseChange));
  Config::Connect (oss2.str (), MakeCallback (&phyMonitorSniffRx));
  
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
