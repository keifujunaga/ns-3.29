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

#define MOB_NUM 15
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
              apps1.Start (Seconds (APPSTARTSEC));
              apps1.Stop (Seconds (APPSTARTSEC + 5.0));
            }
          }
      }
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
      nodelist[i] = -1;
    }

  nodelist[0] = 1;
  nodelist[1] = 1;
  nodelist[4] = 1;
  nodelist[5] = 1;
  nodelist[6] = 1;
  nodelist[7] = 1;
  nodelist[9] = 1;
  nodelist[12] = 1;

  // 1. Create x nodes
  mobilenode.Create (MOB_NUM);

  /*
   * SET MOBILE MOBILITY
   */
  Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator> ();
  initialAlloc->Add (Vector (-232.173, 361.674, 1.500));
  initialAlloc->Add (Vector (-141.198, 247.309, 1.500));
  initialAlloc->Add (Vector (-311.157, 216.237, 1.500));
  initialAlloc->Add (Vector (127.472, 292.812, 1.500));
  initialAlloc->Add (Vector (-287.132, 128.524, 1.500));
  initialAlloc->Add (Vector (-39.917, 156.259, 1.500));
  initialAlloc->Add (Vector (323.176, 203.984, 1.500));
  initialAlloc->Add (Vector (261.370, 87.408, 1.500));
  initialAlloc->Add (Vector (103.427, 64.099, 1.500));
  initialAlloc->Add (Vector (-197.868, 40.804, 1.500));
  initialAlloc->Add (Vector (-302.626, -286.714, 1.500));
  initialAlloc->Add (Vector (-90.596, -292.284, 1.500));
  initialAlloc->Add (Vector (31.297, -348.916, 1.500));
  initialAlloc->Add (Vector (-168.738, -590.939, 1.500));
  initialAlloc->Add (Vector (123.145, -426.640, 1.500));
  mobilemobility.SetPositionAllocator (initialAlloc);

  //ue.mob file open
  //waylist
  struct mob_list wpl[MOB_NUM][100] = {}; //
  int jc[MOB_NUM] = {};
  int i = 0;
  std::ifstream ifss ("./routes-mobility-model-considering-olsr.mob");
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
      Ptr<WaypointMobilityModel> waypoint =
          DynamicCast<WaypointMobilityModel> (mobilenode.Get (u)->GetObject<MobilityModel> ());
      for (int v = 0; v < jc[u]; v++)
        {
          waypoint->AddWaypoint (
              Waypoint (NanoSeconds (wpl[u][v].waypoint_t),
                        Vector (wpl[u][v].waypoint_x + 500, wpl[u][v].waypoint_y - 29000, 1.5)));
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

  AnimationInterface anim ("35-calc-reveivetime-downlink-uav-to-mobile.xml");

  for (uint32_t i = 0; i < mobilenode.GetN (); ++i)
    {
      anim.UpdateNodeDescription (mobilenode.Get (i), "MOB"); // Optional
      anim.UpdateNodeColor (mobilenode.Get (i), 0, 255, 0); // Optional
    }

  //Config::Connect ("NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));

  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (
      ascii.CreateFileStream ("35-calc-reveivetime-downlink-uav-to-mobile.mob"));

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