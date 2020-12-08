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

#include <boost/algorithm/string.hpp>
#include <string>
#include <list>
#include <iostream>
#include <boost/foreach.hpp>

#define SHELL_NUM 15
#define MOB_NUM 6
#define UAV_NUM 30

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("EpidemicExample");

int nodeappcount[30] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

double dtnappstart = 0.0;
double dtnappstop = 5.0;
double appTotalTime = 700; // simulator stop time
uint32_t packetSize = 2048; //ugoku
//uint32_t packetSize = 64; // ugoku
//std::string rate = "0.128kbps"; //ugoku
//std::string rate = "0.512kbps";
std::string rate = "4.096kbps"; //ugoku
//std::string rate = "8.192kbps";
//std::string rate = "16.384kbps";

NodeContainer AllNodeContainer;
NodeContainer dtnContainer;
NodeContainer uav;
NodeContainer uavgw;
NodeContainer mobilenode;
NetDeviceContainer devices;
NetDeviceContainer devices2;
Ipv4InterfaceContainer interfaces;
Ipv4InterfaceContainer interfaces2;
ApplicationContainer dtn_apps_sink;

void
CourseChange (std::string context, Ptr<const MobilityModel> model)
{
  Vector position = model->GetPosition ();
  NS_LOG_UNCOND ("Simulation time " << Simulator::Now ().GetSeconds ());
  NS_LOG_UNCOND (context << " x = " << position.x << ", y = " << position.y
                         << ", z = " << position.z);
}

void
ReceivedPacket (std::string context, Ptr<const Packet> packet, const Address &)
{
  /*
   * UAV空中基地局がUAVBSからパケットを受信したら、UAV空中基地局のGWがモバイル端末へパケットを送信する
   * （各モバイル端末へUnicastを使用)
   */
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); //NodeNo

  if ((1 <= kym) && (kym <= 29) && nodeappcount[kym] == 0)
    {
      for (int i = 0; i < MOB_NUM; i++)
        {
          OnOffHelper dtnonoff (
              "ns3::UdpSocketFactory",
              Address (InetSocketAddress (interfaces.GetAddress (UAV_NUM + i), 80)));
          dtnonoff.SetConstantRate (DataRate (rate));
          dtnonoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
          ApplicationContainer dtnapps = dtnonoff.Install (dtnContainer.Get (0));
          dtnapps.Start (Seconds (dtnappstart));
          dtnapps.Stop (Seconds (dtnappstop));
        }
      std::cout << *itr << ", " << Simulator::Now ().GetSeconds () << std::endl;
      nodeappcount[kym]++;
    }
}

void
ReceivedPacketWithAddress (std::string context, Ptr<const Packet> packet, const Address &srcAddress, const Address &destAddress)
{
  /*
   * UAV空中基地局がUAVBSからパケットを受信したら、UAV空中基地局のGWがモバイル端末へパケットを送信する
   * （各モバイル端末へUnicastを使用)
   */
  /*
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); //NodeNo
  */

  std::cout << "srcAddress.GetLength() is " << srcAddress.GetLength() << std::endl;
  std::cout << "srcAddress.GetSerializedSize() is " << srcAddress.GetSerializedSize() << std::endl;
  std::cout << "srcAddress.IsInvalid() is " << srcAddress.IsInvalid() << std::endl;
  std::cout << "srcAddress.MAX_SIZE is " << srcAddress.MAX_SIZE << std::endl;
  std::cout << "srcAddress.Register() is " << srcAddress.Register() << std::endl;
}

void handler (uint32_t i)
{
  std::cout << Simulator::Now().GetSeconds() << std::endl;
  for (int j = 0; j < UAV_NUM; j++)
  {
    OnOffHelper uplink_onoff ("ns3::UdpSocketFactory",
                        Address (InetSocketAddress (interfaces2.GetAddress (j), 80)));
    uplink_onoff.SetConstantRate (DataRate (rate));
    uplink_onoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
    uplink_onoff.Install (uav.Get(i));  
  }
}

int
main (int argc, char *argv[])
{
  // General parameters
  std::string mobility_model = "original";
  double nodeSpeed = 1.23; //speed no henkou
  bool app_logging = true;

  // Epidemic parameters
  uint32_t epidemicHopCount = 1000;
  uint32_t epidemicQueueLength = 1000;
  Time epidemicQueueEntryExpireTime = Seconds (700);
  Time epidemicBeaconInterval = Seconds (1);

  // Application parameters

  double appDataStart = 100.0;
  double appDataEnd = 105.0;
  uint32_t randnum = 1;

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
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
      LogComponentEnableAll (LOG_PREFIX_TIME);
      LogComponentEnableAll (LOG_PREFIX_NODE);
      LogComponentEnableAll (LOG_PREFIX_FUNC);
    }

  LogComponentEnable ("EpidemicExample", LOG_LEVEL_ALL);

  uav.Create (30);
  uavgw.Create (30);
  mobilenode.Create (6);
  dtnContainer.Add (uavgw);
  dtnContainer.Add (mobilenode);

  AllNodeContainer.Add (uav);
  AllNodeContainer.Add (uavgw);
  AllNodeContainer.Add (mobilenode);

  /*
  *     ダミーの障害物を作る
  */
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

  /*
   *       Mobility model Setup
   */
  MobilityHelper mobility;
  MobilityHelper uavgwmobility;
  MobilityHelper mobility2;
  MobilityHelper mobility3;
  MobilityHelper mobility4;
  MobilityHelper mobility5;
  MobilityHelper mobility6;
  MobilityHelper mobility7;

  /*
  *  UAVとUAVのゲートウェイの位置をListPositionAllocatorとConstantPositionMobilityで指定
  */
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
  for (int i = 0; i < 30; i++)
    {
      mobility.Install (uav.Get (i));
    }

  uavgwmobility.SetPositionAllocator (initialAlloc);
  uavgwmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  uavgwmobility.Install (uavgw);

  /*
  * モバイル端末の動作をListPositionAllocatorとSteadyStateRandomWayPointMobilityModelで設定
  */
  std::srand (randnum);
  // std::cout << "std::rand() per 213 is " << std::rand()%213 << std::endl;

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

  BuildingsHelper::Install (AllNodeContainer);
  BuildingsHelper::MakeMobilityModelConsistent ();

  /*
   *       Physical and link Layers Setup
   */

  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("Frequency", UintegerValue (2400));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (30));
  wifiPhy.Set ("TxPowerStart", DoubleValue (30));
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::HybridBuildingsPropagationLossModel");

  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi;

  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);

  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold",
                                UintegerValue (0));

  devices = wifi.Install (wifiPhy, wifiMac, uav);
  devices2 = wifi.Install (wifiPhy, wifiMac, dtnContainer);

  /*
   *       Routing Setup
   */
  EpidemicHelper epidemic;
  epidemic.Set ("HopCount", UintegerValue (epidemicHopCount));
  epidemic.Set ("QueueLength", UintegerValue (epidemicQueueLength));
  epidemic.Set ("QueueEntryExpireTime", TimeValue (epidemicQueueEntryExpireTime));
  epidemic.Set ("BeaconInterval", TimeValue (epidemicBeaconInterval));

  //AodvHelper aodv;
  Ipv4StaticRoutingHelper staticRouting;
  OlsrHelper olsr;
  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

  /*
   *       Internet Stack Setup
   */
  InternetStackHelper internet;
  internet.SetRoutingHelper (list);
  internet.Install (uav);
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
   *         UAVBSが各UAV空中基地局に対してブロードキャストを使用してパケットを送信する
   */
  PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
  ApplicationContainer apps_sink = sink.Install (uav);
  apps_sink.Start (Seconds (0.0));
  apps_sink.Stop (Seconds (appTotalTime));

  // Client setup
  OnOffHelper onoff1 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (Ipv4Address ("192.168.1.255"), 80)));
  onoff1.SetConstantRate (DataRate (rate));
  onoff1.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps1 = onoff1.Install (uav.Get (0));
  apps1.Start (Seconds (appDataStart));
  apps1.Stop (Seconds (appDataEnd));

  PacketSinkHelper dtnsink ("ns3::UdpSocketFactory",
                            InetSocketAddress (Ipv4Address::GetAny (), 80));
  dtn_apps_sink = dtnsink.Install (mobilenode);
  dtn_apps_sink.Start (Seconds (0.0));
  dtn_apps_sink.Stop (Seconds (appTotalTime));

  Simulator::Stop (Seconds (appTotalTime));

  // netanim
  AnimationInterface anim ("wireless-animation.xml"); // Mandatory

  // iro to moji no settei

  for (uint32_t i = 0; i < uav.GetN (); ++i)
    {
      anim.UpdateNodeDescription (uav.Get (i), "UAVGW"); // Optional
      anim.UpdateNodeColor (uav.Get (i), 255, 0, 0); // Optional
    }

  // anim.EnablePacketMetadata (); // Optional
  anim.EnableIpv4RouteTracking ("dicomo-jikken-olsr-and-epid-routingtable.xml", Seconds (100),
                                Seconds (200),
                                Seconds (0.5)); //Optional
  // anim.EnableWifiMacCounters (Seconds (0), Seconds (600)); //Optional
  // anim.EnableWifiPhyCounters (Seconds (0), Seconds (600)); //Optional

  //anim.SetMaxPktsPerTraceFile	(10000000);

  //Config::Connect ("NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));
  Config::Connect ("NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",
                   MakeCallback (&ReceivedPacket));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
