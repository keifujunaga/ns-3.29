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

#include <boost/algorithm/string.hpp>
#include <string>
#include <list>
#include <iostream>
#include <boost/foreach.hpp>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

#define UAV_NUM 60
#define SECINTV 1
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
MobilityHelper mobility;

 // Vector velo = Vector (1.0, 1.0, 200.0);

int testcount;
double testcount2;
int testcount3;
double seccount = 0;
int mobcount = 0;

// 1. Create x nodes
NodeContainer uav;

double
PositionToDistance (Vector a, Vector b)
{
  return sqrt (pow ((a.x - b.x), 2) + pow ((a.y - b.y), 2));
}

bool
setVirtualSpring (int i, int j)
{
  Vector O;
  if (i == j)
    return false;

  for (int k = 0; k < UAV_NUM; k++)
    {
      if (k != i && k != j)
        {
          O.x = (nodePs[i].x + nodePs[j].x) / 2; // tyuuten
          O.y = (nodePs[i].y + nodePs[j].y) / 2; // tyuuten

          if ((PositionToDistance (nodePs[i], nodePs[j]) / 2) >= PositionToDistance (nodePs[k], O))
            {
              return false;
            }
        }
    }
  return true;
}

void
setUnitVector ()
{
  // NS_LOG_UNCOND("Time is " << Simulator::Now());
  for (int i = 0; i < UAV_NUM; i++)
    {
      for (int j = 0; j < UAV_NUM; j++)
        {
          li[i][j] = PositionToDistance (nodePs[i], nodePs[j]);
          if (cRange - li[i][j] > 0 && i != j && setVirtualSpring (i, j) == true)
            {
              unitvector[i][j].x = (nodePs[j].x - nodePs[i].x) / li[i][j];
              unitvector[i][j].y = (nodePs[j].y - nodePs[i].y) / li[i][j];
              acceleration[i].x += kAtA * (li[i][j] - lo) * unitvector[i][j].x;
              acceleration[i].y += kAtA * (li[i][j] - lo) * unitvector[i][j].y;
            }
          else
            {
              unitvector[i][j].x = 0.0;
              unitvector[i][j].y = 0.0;
            }
        }
    }
}

void
AccelerationToVelocityAndPosition ()
{
  NS_LOG_UNCOND("Time is " << Simulator::Now());
  double testX, testY, nextpositionX, nextpositionY, nextvelocityX, nextvelocityY, vel, x;
  for (int i = 0; i < UAV_NUM; i++)
    {
      nextvelocityX = acceleration[i].x * SECINTV;
      nextvelocityY = acceleration[i].y * SECINTV;
      nextpositionX = nextvelocityX * SECINTV / 2;
      nextpositionY = nextvelocityY * SECINTV / 2;
      testX = nodePs[i].x + nextpositionX;
      testY = nodePs[i].y + nextpositionY;
      nodePs[i].z = 200;
      if ((-constraintareaX <= testX && testX <= constraintareaX) && (-constraintareaY <= testY && testY <= constraintareaY))
        {
          velocity[i].x = nextvelocityX;
          velocity[i].y = nextvelocityY;
          vel = std::sqrt(std::pow(velocity[i].x, 2)+std::pow(velocity[i].y, 2));
          if (vel > VELOTHOLD){
            x = vel / VELOTHOLD;
            velocity[i].x = velocity[i].x / x;
            velocity[i].y = velocity[i].y / x; 
          }
          nodePs[i].x = testX;
          nodePs[i].y = testY;
          // NS_LOG_UNCOND("Velocity[" << i << "] = " << velocity[i]);
          std::cout << "nodePs[" << i << "] = " << nodePs[i] << std::endl;
        }
    }
}

void
resetVariable ()
{
  // NS_LOG_UNCOND("Time is " << Simulator::Now());
  for (int i = 0; i < UAV_NUM; i++)
    {
      acceleration[i].x = 0.0;
      acceleration[i].y = 0.0;
      for (int j = 0; j < UAV_NUM; j++)
        {
          unitvector[i][j].x = 0.0;
          unitvector[i][j].y = 0.0;
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

  int nodenum = 0;

  LogComponentEnable ("FirstScriptExample", LOG_LEVEL_ALL);

  /*
   * SET UAV MOBILITY  
   */

  // 1. Create x nodes
  uav.Create (UAV_NUM);

  // 2. Set mobility model and place nodes
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator", "MinX", DoubleValue (0), "MinY",
                                 DoubleValue (0), "Z", DoubleValue (0), "DeltaX",
                                 DoubleValue (5.0), "DeltaY", DoubleValue (5.0), "GridWidth",
                                 UintegerValue (7), "LayoutType", StringValue ("RowFirst"));

  // mobility.SetMobilityModel("ns3::VirtualSpringMobilityModel");
  mobility.SetMobilityModel ("ns3::WaypointMobilityModel");
  mobility.Install (uav);

  // 3. set initial position to nodePs
  for (NodeContainer::Iterator i = uav.Begin (); i != uav.End (); ++i)
    {
      Ptr<Node> node = *i;
      std::string name =
          Names::FindName (node); // Assume that nodes are named, remove this line otherwise
      Ptr<MobilityModel> mob = node->GetObject<MobilityModel> ();
      if (!mob)
        continue; // Strange -- node has no mobility model installed. Skip.
      nodePs[nodenum] = mob->GetPosition ();
      // vsm->SetVSMVelocity(velo);
      nodenum++;
    }

  // 4. Update node Velocity every SECINTV Seconds until simulator has stop.
  for (double i = 0; i < APPSTOPSEC; i += SECINTV)
    {
      mobcount++;
    }

  Vector nodePsContainer[UAV_NUM][mobcount];

  mobcount = 0;

  for (double i = 0; i < APPSTOPSEC; i += SECINTV)
    {
      setUnitVector ();
      AccelerationToVelocityAndPosition ();
      resetVariable ();
      for (int u = 0; u < UAV_NUM; u++)
      {
        nodePsContainer[u][mobcount] = nodePs[u];
      }
      mobcount++;
    }

  for (int u = 0; u < UAV_NUM; u++)
    {
      mobcount = 0;
      Ptr<WaypointMobilityModel> waypoint =
          DynamicCast<WaypointMobilityModel> (uav.Get (u)->GetObject<MobilityModel> ());
      for (double v = 0; v < APPSTOPSEC; v += SECINTV)
        {
          waypoint->AddWaypoint (Waypoint (Seconds (v), nodePsContainer[u][mobcount]));
          mobcount++;
        }
      std::cout << "maximum mobcount is " << mobcount << std::endl;
    }

  /*
   * SET MOBILE MOBILITY
   */

  NodeContainer mob;
  mob.Create(MOB_NUM);

  NodeContainer shell;
  shell.Create(SHELL_NUM);

  NodeContainer mobshel;
  mobshel.Add(mob);
  mobshel.Add(shell);

  Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator> ();
  initialAlloc->Add (Vector (-232.173,361.674,1.500)); 
  initialAlloc->Add (Vector (-141.198,247.309,1.500)); 
  initialAlloc->Add (Vector (-311.157,216.237,1.500)); 
  initialAlloc->Add (Vector (127.472,292.812,1.500)); 
  initialAlloc->Add (Vector (-287.132,128.524,1.500)); 
  initialAlloc->Add (Vector (-39.917,156.259,1.500)); 
  initialAlloc->Add (Vector (323.176,203.984,1.500)); 
  initialAlloc->Add (Vector (261.370,87.408,1.500)); 
  initialAlloc->Add (Vector (103.427,64.099,1.500)); 
  initialAlloc->Add (Vector (-197.868,40.804,1.500)); 
  initialAlloc->Add (Vector (-302.626,-286.714,1.500)); 
  initialAlloc->Add (Vector (-90.596,-292.284,1.500)); 
  initialAlloc->Add (Vector (31.297,-348.916,1.500)); 
  initialAlloc->Add (Vector (-168.738,-590.939,1.500)); 
  initialAlloc->Add (Vector (123.145,-426.640,1.500)); 
  initialAlloc->Add (Vector (-160.124,-253.419,1.500)); 
  initialAlloc->Add (Vector (-121.482,-84.662,1.500)); 
  initialAlloc->Add (Vector (-300.022,-10.258,1.500)); 
  initialAlloc->Add (Vector (214.157,33.006,1.500)); 
  initialAlloc->Add (Vector (419.312,55.207,1.500)); 
  initialAlloc->Add (Vector (406.437,175.116,1.500)); 
  initialAlloc->Add (Vector (281.974,196.213,1.500)); 
  initialAlloc->Add (Vector (-149.832,-353.344,1.500)); 
  initialAlloc->Add (Vector (-157.496,369.440,1.500)); 
  initialAlloc->Add (Vector (-432.252,-322.228,1.500)); 
  initialAlloc->Add (Vector (-565.246,129.667,1.500)); 
  initialAlloc->Add (Vector (219.319,348.322,1.500)); 
  initialAlloc->Add (Vector (551.500,469.336,1.500)); 
  initialAlloc->Add (Vector (-236.440,624.808,1.500)); 
  initialAlloc->Add (Vector (-468.174,723.647,1.500)); 
  mobility.SetPositionAllocator (initialAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(mobshel);

//NodeContainer all;
//all.Add(uav);
//all.Add(mob);

  Simulator::Stop (Seconds (APPSTOPSEC));

  AnimationInterface anim ("2-cover-uav-using-vsm.xml");

  for (uint32_t i = 0; i < uav.GetN (); ++i)
    {
      anim.UpdateNodeDescription (uav.Get (i), "UAV"); // Optional
      anim.UpdateNodeColor (uav.Get (i), 255, 0, 0); // Optional
    }

  for (uint32_t i = 0; i < mob.GetN (); ++i)
    {
      anim.UpdateNodeDescription (mob.Get (i), "MOB"); // Optional
      anim.UpdateNodeColor (mob.Get (i), 0, 255, 0); // Optional
    }
  
  for (uint32_t i = 0; i < shell.GetN (); ++i)
    {
      anim.UpdateNodeDescription (shell.Get (i), "SHELL"); // Optional
      anim.UpdateNodeColor (shell.Get (i), 0, 0, 255); // Optional
    }
  

  //Config::Connect ("NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));

  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("2-cover-uav-using-vsm.mob"));

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