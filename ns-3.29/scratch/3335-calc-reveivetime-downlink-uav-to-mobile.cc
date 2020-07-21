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
#define APPSTOPSEC 1500
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
NodeContainer mobilenode;

std::string rate = "4.096kbps";
uint32_t packetSize = 2048;
Ipv4InterfaceContainer interfaces;
ApplicationContainer downlink_apps;
NetDeviceContainer devices;
MobilityHelper mobilemobility;

// Epidemic parameters
uint32_t epidemicHopCount = 1000;
uint32_t epidemicQueueLength = 1000;
Time epidemicQueueEntryExpireTime = Seconds (APPSTOPSEC);
Time epidemicBeaconInterval = Seconds (1);

struct mob_list
{ // zahyou no hensu
  int node_no;
  double waypoint_t;
  double waypoint_x;
  double waypoint_y;
  double waypoint_z;
};

int nodelist[MOB_NUM];
int delaylist[MOB_NUM];
struct mob_list wpl[MOB_NUM][100] = {}; //
int jc[MOB_NUM] = {};

void
down_handler ()
{
  std::cout << "down_handler is started at " << Simulator::Now ().GetSeconds () << std::endl;
  for (int i = 0; i < MOB_NUM; i++)
    {
      if (nodelist[i] == 1)
        {
          for (int j = 0; j < MOB_NUM; j++)
            {
              if (nodelist[j] == -1)
                {
                  OnOffHelper onoff1 ("ns3::UdpSocketFactory",
                                      Address (InetSocketAddress (interfaces.GetAddress (j), 80)));
                  onoff1.SetConstantRate (DataRate (rate));
                  onoff1.SetAttribute ("PacketSize", UintegerValue (packetSize));
                  ApplicationContainer apps1 = onoff1.Install (mobilenode.Get (i));
                  apps1.Start (Seconds (APPSTARTSEC + delaylist[i]));
                  apps1.Stop (Seconds (APPSTARTSEC + delaylist[i] + 5.0));
                }
            }
        }
    }
}

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
  if (nodelist[kym] == -1)
    {
      std::cout << "node" << kym << " is received at " << Simulator::Now().GetSeconds() << std::endl; 
      Ptr<WaypointMobilityModel> waypoint =
          DynamicCast<WaypointMobilityModel> (mobilenode.Get (kym)->GetObject<MobilityModel> ());
      for (int v = 0; v < jc[kym]; v++)
        {
          waypoint->AddWaypoint (
              Waypoint (NanoSeconds (wpl[kym][v].waypoint_t+Simulator::Now().GetNanoSeconds()),
                        Vector (wpl[kym][v].waypoint_x, wpl[kym][v].waypoint_y, 1.5)));
        }
      nodelist[kym] = 1;
    }
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

  for (int i = 0; i < MOB_NUM; i++)
    {
      nodelist[i] = 1;
    }

  nodelist[0] = -1;
  nodelist[1] = -1;
  nodelist[5] = -1;
  nodelist[6] = -1;
  nodelist[8] = -1;
  nodelist[14] = -1;
  nodelist[15] = -1;
  nodelist[16] = -1;
  nodelist[19] = -1;
  nodelist[20] = -1;
  nodelist[22] = -1;
  nodelist[23] = -1;
  nodelist[25] = -1;
  nodelist[26] = -1;
  nodelist[27] = -1;
  nodelist[30] = -1;
  nodelist[31] = -1;
  nodelist[32] = -1;
  nodelist[33] = -1;  
  nodelist[37] = -1;
  nodelist[38] = -1;
  nodelist[39] = -1;
  nodelist[40] = -1;
  nodelist[42] = -1;
  nodelist[43] = -1;

  delaylist[2] = 333.298;
  delaylist[3] = 333.28;
  delaylist[4] = 333.326;
  delaylist[7] = 333.334;
  delaylist[9] = 334.274;
  delaylist[10] = 333.291;
  delaylist[11] = 334.405;
  delaylist[12] = 334.299;
  delaylist[13] = 335.382;
  delaylist[17] = 337.114;
  delaylist[18] = 335.349;
  delaylist[21] = 333.07;
  delaylist[24] = 334.354;
  delaylist[28] = 336.111;
  delaylist[29] = 334.264;
  delaylist[34] = 333.822;
  delaylist[35] = 337.143;
  delaylist[36] = 334.952;
  delaylist[41] = 333.297;
  delaylist[44] = 333.173;


  // 1. Create x nodes
  mobilenode.Create (MOB_NUM);

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

  //ue.mob file open
  //waylist

  int i = 0;
  std::ifstream ifss ("./4-routes-mobility-to-waypoint-mobility.mob");
  std::string str;
  if (ifss.fail ())
    {
      std::cerr << "失敗" << std::endl;
      return -1;
    }

  while (std::getline (ifss, str))
    {
      std::string delim ("= :");
      std::list<std::string> list_waypointstr;
      boost::split (list_waypointstr, str, boost::is_any_of (delim));
      auto itr2 = list_waypointstr.begin ();
      ++itr2; //waypointtime
      ++itr2; //node
      ++itr2; //nodeno
      i = std::stoi (*itr2);
      wpl[i][jc[i]].node_no = std::stoi (*itr2);
      --itr2; //node
      --itr2; //waypointtime
      std::string ytk = *itr2;
      ytk.erase (--ytk.end ());
      ytk.erase (--ytk.end ());
      wpl[i][jc[i]].waypoint_t = std::stod (ytk);
      ++itr2; //node
      ++itr2; //nodeno
      ++itr2; //pos
      ++itr2; //x
      wpl[i][jc[i]].waypoint_x = std::stod (*itr2);
      ++itr2; //y
      wpl[i][jc[i]].waypoint_y = std::stod (*itr2);
      ++itr2; //z
      wpl[i][jc[i]].waypoint_z = std::stod (*itr2);
      jc[i]++;
    }

  mobilemobility.SetMobilityModel ("ns3::WaypointMobilityModel");
  mobilemobility.Install (mobilenode);

  for (int u = 0; u < MOB_NUM; u++)
    {
      if (nodelist[u] == 1)
        {
          Ptr<WaypointMobilityModel> waypoint =
              DynamicCast<WaypointMobilityModel> (mobilenode.Get (u)->GetObject<MobilityModel> ());
          for (int v = 0; v < jc[u]; v++)
            {
              waypoint->AddWaypoint (
                  Waypoint (NanoSeconds (wpl[u][v].waypoint_t),
                            Vector (wpl[u][v].waypoint_x, wpl[u][v].waypoint_y, 1.5)));
            }
        }
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
                                StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold",
                                UintegerValue (0));

  devices = wifi.Install (wifiPhy, wifiMac, mobilenode);

  /*
   *   Routing Setup
   */

  EpidemicHelper epidemic;
  epidemic.Set ("HopCount", UintegerValue (epidemicHopCount));
  epidemic.Set ("QueueLength", UintegerValue (epidemicQueueLength));
  epidemic.Set ("QueueEntryExpireTime", TimeValue (epidemicQueueEntryExpireTime));
  epidemic.Set ("BeaconInterval", TimeValue (epidemicBeaconInterval));

  /*
   *       Internet Stack Setup
   */
  InternetStackHelper internet;
  internet.SetRoutingHelper (epidemic);
  internet.Install (mobilenode);
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  interfaces = ipv4.Assign (devices);

  /*
   *         Application Setup
   */
  // DOWNLINK
  /// set sink nodes
  ApplicationContainer dtn_sink = sink.Install (mobilenode);
  dtn_sink.Start (Seconds (APPSTARTSEC));
  dtn_sink.Stop (Seconds (APPSTOPSEC));

  /// set onoff node
  Simulator::Schedule (Seconds (0), &down_handler); //329

  /*
   *         other
   */
  Simulator::Stop (Seconds (appTotalTime));

  AnimationInterface anim ("3335-calc-reveivetime-downlink-mobile-to-mobile.xml");

  for (uint32_t i = 0; i < mobilenode.GetN (); ++i)
    {
      anim.UpdateNodeDescription (mobilenode.Get (i), "MOB"); // Optional
      anim.UpdateNodeColor (mobilenode.Get (i), 0, 255, 0); // Optional
    }

  //Config::Connect ("NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));
  Config::Connect ("NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",
                  MakeCallback (&ReceivedPacket));

  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (
      ascii.CreateFileStream ("3335-calc-reveivetime-downlink-mobile-to-mobile.mob"));

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