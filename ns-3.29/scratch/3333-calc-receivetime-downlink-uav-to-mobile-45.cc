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
#include "ns3/mpi-interface.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/virtual-spring-mobility-model.h"
#include "ns3/olsr-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/epidemic-helper.h"

#include <boost/algorithm/string.hpp>
#include <string>
#include <list>
#include <iostream>
#include <boost/foreach.hpp>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

#define UAV_NUM 60
#define SECINTV 1
#define APPSTARTSEC 0
#define APPSTOPSEC 800
#define VELOTHOLD 17 /// [m/s]

#define MOB_NUM 45
#define SHELL_NUM 15

int constraintareaX = 10000;
int constraintareaY = 10000;
int lo = 232;
double kAtA = 0.5;
double speed = 0;
double cRange = 600;
Vector acceleration[UAV_NUM];
Vector velocity[UAV_NUM];
Vector nodePs[UAV_NUM];
double li[UAV_NUM][UAV_NUM];
Vector unitvector[UAV_NUM][UAV_NUM];


// Vector velo = Vector (1.0, 1.0, 200.0);

int testcount;
double testcount2;
int testcount3;
double seccount = 0;
int mobcount = 0;

// 1. Create x nodes

// sinks setting
PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
PacketSinkHelper dtnsink ("ns3::UdpSocketFactory",
                            InetSocketAddress (Ipv4Address::GetAny (), 80));
NodeContainer uav;
NodeContainer mobilenode;
NodeContainer allnode;

std::string rate = "4.096kbps";
uint32_t packetSize = 2048;
Ipv4InterfaceContainer interfaces;
Ipv4InterfaceContainer interfaces2;
ApplicationContainer uplink_apps;
ApplicationContainer downlink_apps;
NetDeviceContainer devices;
NetDeviceContainer devices2;
MobilityHelper mobilemobility;
MobilityHelper uavmobility;
MobilityHelper uavgwmobility;
ApplicationContainer apps1;

// Epidemic parameters
uint32_t epidemicHopCount = 1000;
uint32_t epidemicQueueLength = 1000;
Time epidemicQueueEntryExpireTime = Seconds (APPSTOPSEC);
Time epidemicBeaconInterval = Seconds (1);

/*
void
down_handler ()
{
  std::cout << "down_handler() is started at " << Simulator::Now ().GetSeconds () << std::endl;
  OnOffHelper onoff1 ("ns3::UdpSocketFactory",
                      Address (InetSocketAddress (Ipv4Address ("192.168.1.255"), 80)));
  onoff1.SetConstantRate (DataRate (rate));
  onoff1.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer apps1 = onoff1.Install (uav.Get (0));
  apps1.Start (Seconds (APPSTARTSEC));
  apps1.Stop (Seconds (APPSTARTSEC+5.0));
}
*/

void down_handler (){
  std::cout << "down_handler() is started at " << Simulator::Now ().GetSeconds () << std::endl;
  for (int i = UAV_NUM; i < UAV_NUM + MOB_NUM; i++) {
    OnOffHelper onoff1 ("ns3::UdpSocketFactory",
                        Address (InetSocketAddress (interfaces.GetAddress(i), 80)));
    onoff1.SetConstantRate (DataRate (rate));
    onoff1.SetAttribute ("PacketSize", UintegerValue (packetSize));
    apps1 = onoff1.Install (uav.Get (0));
    apps1.Start (Seconds (APPSTARTSEC));
    apps1.Stop (Seconds (APPSTARTSEC+5.0));
  }
}

void 
time_handler ()
{
  std::cout << "time_handler is " << Simulator::Now().GetSeconds() << std::endl;
}

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

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // int nodenum = 0;
  bool app_logging = true;
  double appTotalTime = APPSTARTSEC + APPSTOPSEC;

  if (app_logging)
    {
      LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
      LogComponentEnableAll (LOG_PREFIX_TIME);
      LogComponentEnableAll (LOG_PREFIX_NODE);
      LogComponentEnableAll (LOG_PREFIX_FUNC);
    }

  LogComponentEnable ("FirstScriptExample", LOG_LEVEL_ALL);

  /*
   * SET UAV MOBILITY  
   */

  // 1. Create x nodes
  uav.Create (UAV_NUM);
  mobilenode.Create (MOB_NUM);
  allnode.Add(uav);
  allnode.Add(mobilenode);

  /*
  *  UAVとUAVのゲートウェイの位置をListPositionAllocatorとConstantPositionMobilityで指定
  */
 Ptr<ListPositionAllocator> uavinitialAlloc = CreateObject<ListPositionAllocator> ();
  uavinitialAlloc->Add(Vector(-471.575,-656.289,200)); 
  uavinitialAlloc->Add(Vector(-243.550,-724.704,200)); 
  uavinitialAlloc->Add(Vector(-72.514,-602.786,200)); 
  uavinitialAlloc->Add(Vector(8.628,-802.443,200)); 
  uavinitialAlloc->Add(Vector(254.060,-838.069,200)); 
  uavinitialAlloc->Add(Vector(423.130,-656.444,200)); 
  uavinitialAlloc->Add(Vector(678.288,-616.541,200)); 
  uavinitialAlloc->Add(Vector(-810.509,-386.478,200)); 
  uavinitialAlloc->Add(Vector(-572.706,-440.761,200)); 
  uavinitialAlloc->Add(Vector(-244.659,-242.953,200)); 
  uavinitialAlloc->Add(Vector(92.374,-454.892,200)); 
  uavinitialAlloc->Add(Vector(339.871,-427.743,200)); 
  uavinitialAlloc->Add(Vector(-130.008,-418.849,200)); 
  uavinitialAlloc->Add(Vector(745.910,-366.931,200)); 
  uavinitialAlloc->Add(Vector(-444.117,-258.885,200)); 
  uavinitialAlloc->Add(Vector(-331.419,218.862,200)); 
  uavinitialAlloc->Add(Vector(-156.725,131.797,200)); 
  uavinitialAlloc->Add(Vector(56.174,95.045,200)); 
  uavinitialAlloc->Add(Vector(281.110,30.565,200)); 
  uavinitialAlloc->Add(Vector(-20.970,304.602,200)); 
  uavinitialAlloc->Add(Vector(545.570,-249.433,200)); 
  uavinitialAlloc->Add(Vector(-737.128,39.959,200)); 
  uavinitialAlloc->Add(Vector(-323.522,-484.155,200)); 
  uavinitialAlloc->Add(Vector(192.705,-287.933,200)); 
  uavinitialAlloc->Add(Vector(-18.300,-256.841,200)); 
  uavinitialAlloc->Add(Vector(701.020,109.340,200)); 
  uavinitialAlloc->Add(Vector(-106.719,-67.179,200)); 
  uavinitialAlloc->Add(Vector(743.108,-112.954,200)); 
  uavinitialAlloc->Add(Vector(-691.208,299.738,200)); 
  uavinitialAlloc->Add(Vector(-526.364,148.060,200)); 
  uavinitialAlloc->Add(Vector(-470.064,382.925,200)); 
  uavinitialAlloc->Add(Vector(382.171,395.260,200)); 
  uavinitialAlloc->Add(Vector(123.429,-99.942,200)); 
  uavinitialAlloc->Add(Vector(544.207,-463.827,200)); 
  uavinitialAlloc->Add(Vector(777.211,332.152,200)); 
  uavinitialAlloc->Add(Vector(-885.246,-153.273,200)); 
  uavinitialAlloc->Add(Vector(-335.583,-22.352,200)); 
  uavinitialAlloc->Add(Vector(403.690,179.775,200)); 
  uavinitialAlloc->Add(Vector(521.150,-4.465,200)); 
  uavinitialAlloc->Add(Vector(183.592,-632.743,200)); 
  uavinitialAlloc->Add(Vector(354.732,-173.717,200)); 
  uavinitialAlloc->Add(Vector(574.449,511.570,200)); 
  uavinitialAlloc->Add(Vector(-673.376,-214.851,200)); 
  uavinitialAlloc->Add(Vector(-541.091,-60.730,200)); 
  uavinitialAlloc->Add(Vector(-35.603,503.790,200)); 
  uavinitialAlloc->Add(Vector(167.556,456.596,200)); 
  uavinitialAlloc->Add(Vector(-152.703,661.924,200)); 
  uavinitialAlloc->Add(Vector(332.344,609.704,200)); 
  uavinitialAlloc->Add(Vector(899.518,99.521,200)); 
  uavinitialAlloc->Add(Vector(-616.320,546.116,200)); 
  uavinitialAlloc->Add(Vector(-380.015,603.761,200)); 
  uavinitialAlloc->Add(Vector(-235.835,427.319,200)); 
  uavinitialAlloc->Add(Vector(571.418,268.337,200)); 
  uavinitialAlloc->Add(Vector(96.292,670.075,200)); 
  uavinitialAlloc->Add(Vector(202.614,248.475,200)); 
  uavinitialAlloc->Add(Vector(487.622,751.946,200)); 
  uavinitialAlloc->Add(Vector(-521.724,772.319,200)); 
  uavinitialAlloc->Add(Vector(-23.614,857.225,200)); 
  uavinitialAlloc->Add(Vector(-283.871,831.303,200)); 
  uavinitialAlloc->Add(Vector(243.094,831.100,200)); 

  uavmobility.SetPositionAllocator(uavinitialAlloc);
  uavmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  uavmobility.Install (uav);

  /*
   * SET MOBILE MOBILITY
   */
  Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator> ();
  initialAlloc->Add(Vector(-232.173,361.674,1.500)); 
  initialAlloc->Add(Vector(-141.198,247.309,1.500)); 
  initialAlloc->Add(Vector(-311.157,216.237,1.500)); 
  initialAlloc->Add(Vector(127.472,292.812,1.500)); 
  initialAlloc->Add(Vector(-287.132,128.524,1.500)); 
  initialAlloc->Add(Vector(-43.351,156.260,1.500)); 
  initialAlloc->Add(Vector(323.176,203.984,1.500)); 
  initialAlloc->Add(Vector(259.653,85.187,1.500)); 
  initialAlloc->Add(Vector(103.427,64.099,1.500)); 
  initialAlloc->Add(Vector(-197.868,40.804,1.500)); 
  initialAlloc->Add(Vector(-302.625,-284.493,1.500)); 
  initialAlloc->Add(Vector(-90.596,-292.284,1.500)); 
  initialAlloc->Add(Vector(31.297,-348.916,1.500)); 
  initialAlloc->Add(Vector(-168.738,-590.939,1.500)); 
  initialAlloc->Add(Vector(123.145,-426.640,1.500)); 
  initialAlloc->Add(Vector(-57.075,282.831,1.500)); 
  initialAlloc->Add(Vector(144.640,296.142,1.500)); 
  initialAlloc->Add(Vector(-205.599,-14.708,1.500)); 
  initialAlloc->Add(Vector(-27.911,0.821,1.500)); 
  initialAlloc->Add(Vector(-37.346,98.525,1.500)); 
  initialAlloc->Add(Vector(30.467,108.513,1.500)); 
  initialAlloc->Add(Vector(335.192,152.912,1.500)); 
  initialAlloc->Add(Vector(2.125,-130.192,1.500)); 
  initialAlloc->Add(Vector(-313.719,337.257,1.500)); 
  initialAlloc->Add(Vector(-449.364,137.424,1.500)); 
  initialAlloc->Add(Vector(-318.069,-211.214,1.500)); 
  initialAlloc->Add(Vector(271.666,-82.463,1.500)); 
  initialAlloc->Add(Vector(335.192,151.801,1.500)); 
  initialAlloc->Add(Vector(273.393,297.248,1.500)); 
  initialAlloc->Add(Vector(175.537,203.989,1.500)); 
  initialAlloc->Add(Vector(80.236,-189.041,1.500)); 
  initialAlloc->Add(Vector(113.724,6.364,1.500)); 
  initialAlloc->Add(Vector(-135.192,206.229,1.500)); 
  initialAlloc->Add(Vector(-287.117,270.638,1.500)); 
  initialAlloc->Add(Vector(-404.731,115.214,1.500)); 
  initialAlloc->Add(Vector(-201.314,-91.317,1.500)); 
  initialAlloc->Add(Vector(-262.259,-77.988,1.500)); 
  initialAlloc->Add(Vector(-301.771,-324.463,1.500)); 
  initialAlloc->Add(Vector(42.493,261.729,1.500)); 
  initialAlloc->Add(Vector(219.304,-50.264,1.500)); 
  initialAlloc->Add(Vector(378.108,-23.622,1.500)); 
  initialAlloc->Add(Vector(-142.932,33.028,1.500)); 
  initialAlloc->Add(Vector(-309.404,555.979,1.500));
  initialAlloc->Add(Vector(-398.686,426.087,1.500)); 
  initialAlloc->Add(Vector(-245.043,421.630,1.500)); 
  mobilemobility.SetPositionAllocator (initialAlloc);
  mobilemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilemobility.Install (mobilenode);

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
  wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");

  wifiPhy.SetChannel (wifiChannel.Create ());

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a); 

  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                StringValue ("OfdmRate6Mbps"), 
                                "RtsCtsThreshold", UintegerValue (0));

  devices = wifi.Install (wifiPhy, wifiMac, allnode);

  /*
   *   Routing Setup
   */

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
   *       Internet Stack Setup
   */
  InternetStackHelper internet;
  internet.SetRoutingHelper (list);
  internet.Install (allnode);
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  interfaces = ipv4.Assign (devices);

  /*
   *         Application Setup
   */
  // DOWNLINK
  /// set sink nodes
  ApplicationContainer dtn_sink = sink.Install(mobilenode);
  dtn_sink.Start (Seconds (APPSTARTSEC));
  dtn_sink.Stop (Seconds (APPSTOPSEC));

  /// set onoff node
  Simulator::Schedule (Seconds (329), &down_handler); //329

  for (int i = 0; i < 1500; i = i + 10)
  {
    Simulator::Schedule(Seconds(i), &time_handler);
  }

  /*
   *         other
   */
  Simulator::Stop (Seconds (appTotalTime));

  AnimationInterface anim ("3333-calc-reveivetime-downlink-uav-to-mobile-45.xml");

  for (uint32_t i = 0; i < uav.GetN (); ++i)
    {
      anim.UpdateNodeDescription (uav.Get (i), "UAV"); // Optional
      anim.UpdateNodeColor (uav.Get (i), 255, 0, 0); // Optional
    }

  for (uint32_t i = 0; i < mobilenode.GetN (); ++i)
    {
      anim.UpdateNodeDescription (mobilenode.Get (i), "MOB"); // Optional
      anim.UpdateNodeColor (mobilenode.Get (i), 0, 255, 0); // Optional
    }

  //Config::Connect ("NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));

  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("3333-calc-reveivetime-downlink-uav-to-mobile-45.mob"));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

/*
now=+0.0ns node=0 pos=-232.173:361.674:1.500 vel=0.265:-1.374:0.000
now=+0.0ns node=1 pos=-141.198:247.309:1.500 vel=-1.296:-0.218:0.000
now=+0.0ns node=2 pos=-311.157:216.237:1.500 vel=-1.278:-0.247:0.000
now=+0.0ns node=3 pos=127.472:292.812:1.500 vel=1.321:0.256:0.000
now=+0.0ns node=4 pos=-287.132:128.524:1.500 vel=1.317:0.179:0.000
now=+0.0ns node=5 pos=-39.917:156.259:1.500 vel=1.320:0.161:0.000
now=+0.0ns node=6 pos=323.176:203.984:1.500 vel=-1.421:-0.268:0.000
now=+0.0ns node=7 pos=261.370:87.408:1.500 vel=-1.034:-0.892:0.000
now=+0.0ns node=8 pos=103.427:64.099:1.500 vel=0.000:1.266:0.000
now=+0.0ns node=9 pos=-197.868:40.804:1.500 vel=-1.314:-0.134:0.000
now=+0.0ns node=10 pos=-302.626:-286.714:1.500 vel=-0.203:1.317:0.000
now=+0.0ns node=11 pos=-90.596:-292.284:1.500 vel=-0.078:1.312:0.000
now=+0.0ns node=12 pos=31.297:-348.916:1.500 vel=1.367:0.000:0.000
now=+0.0ns node=13 pos=-168.738:-590.939:1.500 vel=-1.352:-0.291:0.000
now=+0.0ns node=14 pos=123.145:-426.640:1.500 vel=-0.050:1.437:0.000

now=+719000000000.0ns node=0 pos=-160.124:-253.419:1.500 vel=0.000:0.000:0.000
now=+340000000000.0ns node=1 pos=-121.482:-84.662:1.500 vel=0.000:0.000:0.000
now=+264000000000.0ns node=2 pos=-300.022:-10.258:1.500 vel=0.000:0.000:0.000
now=+243000000000.0ns node=3 pos=214.157:33.006:1.500 vel=0.000:0.000:0.000
now=+642000000000.0ns node=4 pos=419.312:55.207:1.500 vel=0.000:0.000:0.000
now=+361999999999.0ns node=5 pos=406.437:175.116:1.500 vel=0.000:0.000:0.000
now=+29000000000.0ns node=6 pos=281.974:196.213:1.500 vel=0.000:0.000:0.000
now=+644000000000.0ns node=7 pos=-149.832:-353.344:1.500 vel=0.000:0.000:0.000
now=+404000000000.0ns node=8 pos=-157.496:369.440:1.500 vel=0.000:0.000:0.000
now=+493000000000.0ns node=9 pos=-432.252:-322.228:1.500 vel=0.000:0.000:0.000
now=+515000000000.0ns node=10 pos=-565.246:129.667:1.500 vel=0.000:0.000:0.000
now=+755999999999.0ns node=11 pos=219.319:348.322:1.500 vel=0.000:0.000:0.000
now=+1027999999999.0ns node=12 pos=551.500:469.336:1.500 vel=0.000:0.000:0.000
now=+1144999999999.0ns node=13 pos=-236.440:624.808:1.500 vel=0.000:0.000:0.000
now=+1258000000000.0ns node=14 pos=-468.174:723.647:1.500 vel=0.000:0.000:0.000
*/

// trash candidates
// GridPositionAllocatorの使い方
/* 
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue (-100.0),
                                  "MinY", DoubleValue (-100.0),
                                  "DeltaX", DoubleValue (5.0),
                                  "DeltaY", DoubleValue (20.0),
                                  "GridWidth", UintegerValue (20),
                                  "LayoutType", StringValue ("RowFirst"));
*/

// ObjectFactoryを使ったPositionAllocator設定方法
/* 
  ObjectFactory pos;
  pos.SetTypeId ("ns3::GridPositionAllocator");
  pos.Set ("MinX", DoubleValue (-100.0));
  pos.Set ("MinY", DoubleValue (-100.0));   
  pos.Set ("DeltaX", DoubleValue (5.0));
  pos.Set ("DeltaY", DoubleValue (20.0)); 
  pos.Set ("GridWidth", UintegerValue (20.0)); 
  pos.Set ("LayoutType", StringValue ("RowFirst")); 

  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
*/

// ObjectFactoryを使ったPositionAllocator設定方法（RandomRectanglePositionAllocator)
/* 
  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1200.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=900.0]"));
  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
*/

// Simulator::Schedule()を試した際に使用した関数
/* 
void
testschedule ()
{
  testcount++;
  testcount2++;
  testcount3++;
  NS_LOG_UNCOND ("simulation time " << Simulator::Now ().GetSeconds ());
  NS_LOG_UNCOND ("count is " << testcount << "count2 is " << testcount2 << "count3 is "
                             << testcount3);
}
*/