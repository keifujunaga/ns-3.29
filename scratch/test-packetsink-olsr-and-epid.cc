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

#define NUM_UAV 30
#define NUM_MOB 15

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("EpidemicExample");

std::string rate = "0.032kbps";
uint32_t packetSize = 64;
NodeContainer nodeContainer;
Ipv4InterfaceContainer interfaces;
PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));

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
  ss << "test-packetsink-Rx.log";
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

  //f << kym << ", " << packet->GetUid() << ", " << sdb.signal << std::endl;

  /*
  NS_LOG_UNCOND(context << "SIGNAL_POWER = " <<  sdb.signal << "    NOISE_POWER = " << sdb.noise << "   AMPDU_REFNUM = " << mqdu.mpduRefNumber  << "  PKT_SIZE = " << packet->GetSize() << "  " << "   = MonitorSnifferRx");
  */

  /*
  f << "SIMTIME = " << Simulator::Now().GetSeconds() <<" NODE = " << kym << " SIGNAL_POWER = " <<  sdb.signal << " NOISE_POWER = " << sdb.noise << " AMPDU_REFNUM = " << mpdu.mpduRefNumber 
  << " PKT_UID = " << packet->GetUid() <<  " PKT_SIZE = " << packet->GetSize() << " NTx = " << txvec.GetGuardInterval() << std::endl;
*/
}

void
handler (uint32_t i)
{
  std::cout << Simulator::Now ().GetSeconds () << std::endl;
  OnOffHelper uplink_onoff ("ns3::UdpSocketFactory",
                            Address (InetSocketAddress (interfaces.GetAddress (0), 80)));
  uplink_onoff.SetConstantRate (DataRate (rate));
  uplink_onoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
  uplink_onoff.Install (nodeContainer.Get (i + NUM_UAV));
}

int
main (int argc, char *argv[])
{
  // General parameters
  double nodeSpeed = 1.23; //speed no henkou
  bool app_logging = true;
  //std::string rate = "0.512kbps";
  //std::string rate = "2.048kbps";
  // std::string rate = "4.096kbps";
  //uint32_t packetSize = 64;
  //uint32_t packetSize = 2048;
  double appDataStart = 0.0;
  double appDataend = 700.0;
  double simtotaltime = appDataStart + appDataend; // simulator stop time
  uint32_t randnum = 1;

  // Epidemic parameters
  uint32_t epidemicHopCount = 1000;
  uint32_t epidemicQueueLength = 1000;
  Time epidemicQueueEntryExpireTime = Seconds (700);
  Time epidemicBeaconInterval = Seconds (1);

  // arrival time parameter
  /*
  double arrivalTimes[NUM_MOB];
  
  for (int i = 0; i < NUM_MOB; i++){
    arrivalTimes[i] = 100;
  }
  */

  /*
  for (int i = 1; i <= NUM_MOB; i++){
    arrivalTimes[i-1] = std::atof(argv[i]);
    std::cout << arrivalTimes[i-1] << std::endl;
  }
  */

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

  uav.Create (NUM_UAV);
  mobilenode.Create (NUM_MOB);
  nodeContainer.Add (uav);
  nodeContainer.Add (mobilenode);

  /*
   *       Mobility model Setup
   */
  MobilityHelper mobility;
  MobilityHelper mobility2;

  Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator> ();
  for (int i = 0; i < NUM_UAV + NUM_MOB; i++)
    {
      initialAlloc->Add (Vector ((i + 1) * 250, 0, 1.5));
    }

  mobility.SetPositionAllocator (initialAlloc);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodeContainer);

  /*
   *       Physical and link Layers Setup
   */
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  // wifiPhy.Set ("Frequency", UintegerValue (2400));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (30));
  wifiPhy.Set ("TxPowerStart", DoubleValue (30));
  // wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); ///
  //YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();

  YansWifiChannelHelper wifiChannel;

  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel");

  // wifiChannel.AddPropagationLoss ("ns3::ExtendedHataModel");
  // wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel");
  // wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");

  wifiPhy.SetChannel (wifiChannel.Create ());

  WifiHelper wifi;
  // wifi.SetStandard (WIFI_PHY_STANDARD_80211b); ///
  /*
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                               "DataMode",StringValue ("DsssRate1Mbps"),
                               "ControlMode",StringValue ("DsssRate1Mbps"));
  */

  /*
   wifi.SetStandard (WIFI_PHY_STANDARD_80211a); ///
   wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                StringValue ("OfdmRate6Mbps"), ///
                                "RtsCtsThreshold", UintegerValue (60));
                                */

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
  // Sink or server setup

  ApplicationContainer app_sink_test = sink.Install (mobilenode);
  app_sink_test.Start (Seconds (appDataStart));
  app_sink_test.Stop (Seconds (appDataend));

  /*
  for (int i=0; i < NUM_MOB; i++)
  {
    OnOffHelper onoff ("ns3::UdpSocketFactory", 
                      Address (InetSocketAddress (interfaces.GetAddress (NUM_UAV+i), 80))); 
    onoff.SetConstantRate (DataRate (rate));
    onoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
    ApplicationContainer apps = onoff.Install (nodeContainer.Get (0));
    apps.Start (Seconds (100));
    apps.Stop (Seconds (700.0));
  }
  */

  OnOffHelper onoff ("ns3::UdpSocketFactory",
                     Address (InetSocketAddress (interfaces.GetAddress (NUM_UAV+NUM_MOB-1), 80)));
  onoff.SetConstantRate (DataRate (rate));
  onoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps = onoff.Install (nodeContainer.Get (0));
  apps.Start (Seconds (100));
  apps.Stop (Seconds (700.0));

  /*
    OnOffHelper onoff ("ns3::UdpSocketFactory", 
                      Address (InetSocketAddress (interfaces.GetAddress (NUM_UAV+NUM_MOB-1), 80))); 
    onoff.SetConstantRate (DataRate (rate));
    onoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
    ApplicationContainer apps = onoff.Install (nodeContainer.Get (0));
    apps.Start (Seconds (100));
    apps.Stop (Seconds (700.0));
    */

  /*
  /// UPLINK
  ApplicationContainer apps_uplinksink = sink.Install (nodeContainer.Get (0));
  apps_uplinksink.Start (Seconds (appDataStart));
  apps_uplinksink.Stop (Seconds (appDataend));

  for (uint32_t i = 0; i < NUM_MOB; i++) {
    // std::cout << arrivalTimes[i] << std::endl;
    Simulator::Schedule(Seconds(arrivalTimes[i]), &handler, i);
  }
  */

  Simulator::Stop (Seconds (simtotaltime));

  AnimationInterface anim ("test-packetsink-matrix-animation.xml"); // Mandatory

  // std::ostringstream oss;
  std::ostringstream oss2;

  // oss << "/NodeList/*/$ns3::MobilityModel/CourseChange";
  oss2 << "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx";

  // Config::Connect (oss.str (), MakeCallback (&CourseChange));
  Config::Connect (oss2.str (), MakeCallback (&phyMonitorSniffRx));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
