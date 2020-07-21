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
#include <string>

#define SHELL_NUM 15
#define MOB_NUM 6
#define UAV_NUM 30

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

// sinks setting
PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
NodeContainer nodeContainer;
std::string rate = "4.096kbps";
uint32_t packetSize = 2048;
Ipv4InterfaceContainer interfaces;
ApplicationContainer uplink_apps;


void handler (uint32_t i)
{
  std::cout << Simulator::Now().GetSeconds() << std::endl;
  OnOffHelper uplink_onoff ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (0), 80)));
  uplink_onoff.SetConstantRate (DataRate (rate));
  uplink_onoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
  uplink_onoff.Install (nodeContainer.Get(i+UAV_NUM));  
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
  //std::string rate = "4.096kbps";
  //uint32_t packetSize = 64;
  // uint32_t packetSize = 2048;
  double appDataStart = 0.0;
  double appDataEnd = 600.0;
  double appTotalTime = appDataStart + appDataEnd; // simulator stop time
  uint32_t randnum = 1;

  // Epidemic parameters
  uint32_t epidemicHopCount = 1000;
  uint32_t epidemicQueueLength = 1000;
  Time epidemicQueueEntryExpireTime = Seconds (700);
  Time epidemicBeaconInterval = Seconds (1);

  // arrival time parameter
  double arrivalTimes[MOB_NUM];

  for (int i = 1; i < argc; i++){
    arrivalTimes[i-1] = std::atof(argv[i]);
    std::cout << arrivalTimes[i-1] << std::endl;
  }

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


  NodeContainer uav;
  NodeContainer mobilenode;
  NetDeviceContainer devices;

  uav.Create (UAV_NUM);
  mobilenode.Create (MOB_NUM);
  nodeContainer.Add (uav);
  nodeContainer.Add (mobilenode);

  /*
   *       Mobility model Setup
   */
  MobilityHelper mobility;
  MobilityHelper mobility2;
  MobilityHelper mobility3;
  MobilityHelper mobility4;
  MobilityHelper mobility5;
  MobilityHelper mobility6;
  MobilityHelper mobility7;

  double x_min = 0.0;
  double x_max = 10.0;
  double y_min = 0.0;
  double y_max = 10.0;
  double z_min = 0.0;
  double z_max = 10.0;
  Ptr<Building> dammy = CreateObject<Building> ();
  dammy->SetBoundaries (Box (x_min, x_max, y_min, y_max, z_min, z_max));
  dammy->SetBuildingType (Building::Residential);
  dammy->SetExtWallsType (Building::ConcreteWithWindows);
  dammy->SetNFloors (3);
  dammy->SetNRoomsX (3);
  dammy->SetNRoomsY (2);

  Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator> ();
  initialAlloc->Add (Vector (237.807, 145.041, 200));
  initialAlloc->Add (Vector (368.339, 138.992, 200));
  initialAlloc->Add (Vector (499.663, 149.488, 200));
  initialAlloc->Add (Vector (601.48, 173.386, 200));
  initialAlloc->Add (Vector (746.979, 195.871, 200));
  initialAlloc->Add (Vector (183.324, 263.956, 200));
  initialAlloc->Add (Vector (301.872, 245.77, 200));
  initialAlloc->Add (Vector (429.648, 246.436, 200));
  initialAlloc->Add (Vector (547.653, 270.72, 200));
  initialAlloc->Add (Vector (667.221, 292.61, 200));
  initialAlloc->Add (Vector (256.278, 371.906, 200));
  initialAlloc->Add (Vector (364.016, 337.259, 200));
  initialAlloc->Add (Vector (473.152, 370.32, 200));
  initialAlloc->Add (Vector (587.154, 393.942, 200));
  initialAlloc->Add (Vector (709.997, 407.98, 200));
  initialAlloc->Add (Vector (260.268, 498.835, 200));
  initialAlloc->Add (Vector (373.091, 446.186, 200));
  initialAlloc->Add (Vector (497.768, 496.624, 200));
  initialAlloc->Add (Vector (632.006, 507.682, 200));
  initialAlloc->Add (Vector (760.151, 518.776, 200));
  initialAlloc->Add (Vector (265.152, 625.088, 200));
  initialAlloc->Add (Vector (370.534, 559.628, 200));
  initialAlloc->Add (Vector (463.726, 613.423, 200));
  initialAlloc->Add (Vector (572.312, 612.149, 200));
  initialAlloc->Add (Vector (693.685, 620.665, 200));
  initialAlloc->Add (Vector (263.46, 750.206, 200));
  initialAlloc->Add (Vector (373.481, 690.25, 200));
  initialAlloc->Add (Vector (500.886, 724.043, 200));
  initialAlloc->Add (Vector (625.743, 725.584, 200));
  initialAlloc->Add (Vector (750.683, 732.184, 200));
  mobility.SetPositionAllocator (initialAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  // mobility.Install (uav);
  for (int i = 0; i < UAV_NUM; i++)
    {
      mobility.Install (uav.Get (i));
    }

  std::srand (randnum);
  std::cout << "std::rand() per 213 is " << std::rand () % 213 << std::endl;

  Ptr<ListPositionAllocator> initialAlloc2 = CreateObject<ListPositionAllocator> ();
  initialAlloc2->Add (Vector (420, std::rand () % 213 + 420, 1.5));
  Ptr<ListPositionAllocator> initialAlloc3 = CreateObject<ListPositionAllocator> ();
  initialAlloc3->Add (Vector (528, std::rand () % 213 + 420, 1.5));
  Ptr<ListPositionAllocator> initialAlloc4 = CreateObject<ListPositionAllocator> ();
  initialAlloc4->Add (Vector (633, std::rand () % 213 + 420, 1.5));
  Ptr<ListPositionAllocator> initialAlloc5 = CreateObject<ListPositionAllocator> ();
  initialAlloc5->Add (Vector (std::rand () % 213 + 420, 420, 1.5));
  Ptr<ListPositionAllocator> initialAlloc6 = CreateObject<ListPositionAllocator> ();
  initialAlloc6->Add (Vector (std::rand () % 213 + 420, 528, 1.5));
  Ptr<ListPositionAllocator> initialAlloc7 = CreateObject<ListPositionAllocator> ();
  initialAlloc7->Add (Vector (std::rand () % 213 + 420, 633, 1.5));

  mobility2.SetPositionAllocator (initialAlloc2);
  mobility3.SetPositionAllocator (initialAlloc3);
  mobility4.SetPositionAllocator (initialAlloc4);
  mobility5.SetPositionAllocator (initialAlloc5);
  mobility6.SetPositionAllocator (initialAlloc6);
  mobility7.SetPositionAllocator (initialAlloc7);

  mobility2.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX",
                              DoubleValue (415), "MaxX", DoubleValue (425), "MinPause",
                              DoubleValue (0), "MaxPause", DoubleValue (0), "MinY",
                              DoubleValue (420), "MaxY", DoubleValue (633), "Z", DoubleValue (1.5));
  mobility2.Install (mobilenode.Get (0));

  mobility3.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX",
                              DoubleValue (527), "MaxX", DoubleValue (529), "MinPause",
                              DoubleValue (0), "MaxPause", DoubleValue (0), "MinY",
                              DoubleValue (420), "MaxY", DoubleValue (633), "Z", DoubleValue (1.5));
  mobility3.Install (mobilenode.Get (1));

  mobility4.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX",
                              DoubleValue (631), "MaxX", DoubleValue (635), "MinPause",
                              DoubleValue (0), "MaxPause", DoubleValue (0), "MinY",
                              DoubleValue (420), "MaxY", DoubleValue (633), "Z", DoubleValue (1.5));
  mobility4.Install (mobilenode.Get (2));

  mobility5.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX",
                              DoubleValue (420), "MaxX", DoubleValue (633), "MinPause",
                              DoubleValue (0), "MaxPause", DoubleValue (0), "MinY",
                              DoubleValue (415), "MaxY", DoubleValue (425), "Z", DoubleValue (1.5));
  mobility5.Install (mobilenode.Get (3));

  mobility6.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX",
                              DoubleValue (420), "MaxX", DoubleValue (633), "MinPause",
                              DoubleValue (0), "MaxPause", DoubleValue (0), "MinY",
                              DoubleValue (527), "MaxY", DoubleValue (529), "Z", DoubleValue (1.5));
  mobility6.Install (mobilenode.Get (4));

  mobility7.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX",
                              DoubleValue (420), "MaxX", DoubleValue (633), "MinPause",
                              DoubleValue (0), "MaxPause", DoubleValue (0), "MinY",
                              DoubleValue (631), "MaxY", DoubleValue (635), "Z", DoubleValue (1.5));
  mobility7.Install (mobilenode.Get (5));

  BuildingsHelper::Install (nodeContainer);
  BuildingsHelper::MakeMobilityModelConsistent ();
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
  wifiChannel.AddPropagationLoss ("ns3::HybridBuildingsPropagationLossModel");   
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
  interfaces = ipv4.Assign (devices);

  /*
   *         Application Setup
   */
  /*
  // sinks setting
  PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
  */

  // DOWNLINK
  /// set sink nodes
  ApplicationContainer apps_sink;
  for (int i = UAV_NUM; i < UAV_NUM+MOB_NUM; i++){
    apps_sink = sink.Install (nodeContainer.Get(i));
    apps_sink.Start(Seconds(appDataStart));
    apps_sink.Stop (Seconds (appDataEnd));
  }

  /// set onoff nodes
  for (int i = UAV_NUM; i < UAV_NUM+MOB_NUM; i++)
  {
    OnOffHelper onoff ("ns3::UdpSocketFactory", 
                      Address (InetSocketAddress (interfaces.GetAddress (i), 80))); 
    onoff.SetConstantRate (DataRate (rate));
    onoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
    ApplicationContainer apps = onoff.Install (nodeContainer.Get (0));
    apps.Start (Seconds (appDataStart));
    apps.Stop (Seconds (appDataEnd));
  }

  // UPLINK
  /// set sink nodes
  ApplicationContainer apps_uplinksink = sink.Install (nodeContainer.Get (0));
  apps_uplinksink.Start (Seconds (appDataStart));
  apps_uplinksink.Stop (Seconds (appDataEnd));

  /// set onoff nodes
  for (uint32_t i = 0; i < MOB_NUM; i++) {
    std::cout << arrivalTimes[i] << std::endl;
    Simulator::Schedule(Seconds(arrivalTimes[i]), &handler, i);
  }

  uplink_apps.Start (Seconds (appDataStart));
  uplink_apps.Stop (Seconds (appDataEnd));

  Config::Connect ("NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));

  Simulator::Stop (Seconds (appTotalTime));

  AnimationInterface anim ("dicomo-jikken-olsr3-animation.xml"); // Mandatory

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
