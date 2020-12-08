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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("EpidemicExample");

OlsrHelper olsr;

void
CourseChange (std::string context, Ptr<const MobilityModel> model)
{
  Vector position = model->GetPosition ();
  NS_LOG_UNCOND ("Simulation time " << Simulator::Now ().GetSeconds ());
  NS_LOG_UNCOND (context << " x = " << position.x << ", y = " << position.y
                         << ", z = " << position.z);
}

void testtableaddr ()
{
  Ipv4Address table_addr[2];
  olsr.aaa (NodeList::GetNode (0), table_addr);
  for (int i = 0; i < 2; i++)
  {
    std::cout << "table_addr[" << i << "] is " << table_addr[i] << std::endl;
  }
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
  uint32_t numberofnodes = 3;

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
  NetDeviceContainer devices;

  nodeContainer.Create(numberofnodes);

  /*
   *       Mobility model Setup
   */
  MobilityHelper mobility;

  Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator> ();
  initialAlloc->Add (Vector (500, 500, 200));
  initialAlloc->Add (Vector (1500, 500, 200));
  initialAlloc->Add(Vector(2500, 500, 200));

  mobility.SetPositionAllocator (initialAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  // mobility.Install (uav);
  for (int i = 0; i < (int) numberofnodes; i++)
    {
      mobility.Install (nodeContainer.Get (i));
    }

  /*
   *       Physical and link Layers Setup
   */

  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("Frequency", UintegerValue (2400));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (30));
  wifiPhy.Set ("TxPowerStart", DoubleValue (30));
  YansWifiChannelHelper wifiChannel;
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel");
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
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
  ApplicationContainer apps_sink = sink.Install (nodeContainer.Get (2));
  apps_sink.Start (Seconds (0.0));
  apps_sink.Stop (Seconds (appTotalTime));

  // Client setup
  OnOffHelper onoff1 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (interfaces.GetAddress (2), 80)));
  onoff1.SetConstantRate (DataRate (rate));
  onoff1.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps1 = onoff1.Install (nodeContainer.Get (0));
  apps1.Start (Seconds (appDataStart));
  apps1.Stop (Seconds (appDataEnd));

  for (int i = 0; i < appTotalTime; i = i + 30)
  {
    Simulator::Schedule(Seconds(i), &testtableaddr);
  }

  Simulator::Stop (Seconds (appTotalTime));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
