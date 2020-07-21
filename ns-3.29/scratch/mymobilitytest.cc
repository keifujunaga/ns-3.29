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

#define NUMHOST 30
#define SECINTV 1
#define APPSTOPSEC 800
#define VELOTHOLD 17 /// [m/s]

int constraintareaX = 10000;
int constraintareaY = 10000;
int lo = 232;
double kAtA = 0.5;
double speed = 0;
double cRange = 600;
Vector acceleration[NUMHOST];
Vector velocity[NUMHOST];
Vector nodePs[NUMHOST];
double li[NUMHOST][NUMHOST];
Vector unitvector[NUMHOST][NUMHOST];
MobilityHelper mobility;

 // Vector velo = Vector (1.0, 1.0, 200.0);

int testcount;
double testcount2;
int testcount3;
double seccount = 0;
int mobcount = 0;

// 1. Create x nodes
NodeContainer nodes;

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

  for (int k = 0; k < NUMHOST; k++)
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
  for (int i = 0; i < NUMHOST; i++)
    {
      for (int j = 0; j < NUMHOST; j++)
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
  for (int i = 0; i < NUMHOST; i++)
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
  for (int i = 0; i < NUMHOST; i++)
    {
      acceleration[i].x = 0.0;
      acceleration[i].y = 0.0;
      for (int j = 0; j < NUMHOST; j++)
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

  // 1. Create x nodes
  nodes.Create (NUMHOST);

  // 2. Set mobility model and place nodes
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator", "MinX", DoubleValue (300), "MinY",
                                 DoubleValue (0), "Z", DoubleValue (0), "DeltaX",
                                 DoubleValue (5.0), "DeltaY", DoubleValue (5.0), "GridWidth",
                                 UintegerValue (5), "LayoutType", StringValue ("RowFirst"));

  // mobility.SetMobilityModel("ns3::VirtualSpringMobilityModel");
  mobility.SetMobilityModel ("ns3::WaypointMobilityModel");
  mobility.Install (nodes);

  // 3. set initial position to nodePs
  for (NodeContainer::Iterator i = nodes.Begin (); i != nodes.End (); ++i)
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

  // 4. Update node Velocity every 1 Seconds until simulator has stop.
  /*
  for (double i = 0; i < APPSTOPSEC; i += SECINTV)
    {
      Simulator::Schedule (Seconds (i), &setUnitVector);
      Simulator::Schedule (Seconds (i), &AccelerationToVelocityAndPosition);
      Simulator::Schedule (Seconds (i), &resetVariable);
      Simulator::Schedule (Seconds (i), &testvelocity);
      mobcount++;
    }
  */
  for (double i = 0; i < APPSTOPSEC; i += SECINTV)
    {
      mobcount++;
    }

  Vector nodePsContainer[NUMHOST][mobcount];

  mobcount = 0;

  for (double i = 0; i < APPSTOPSEC; i += SECINTV)
    {
      setUnitVector ();
      AccelerationToVelocityAndPosition ();
      resetVariable ();
      for (int u = 0; u < NUMHOST; u++)
      {
        nodePsContainer[u][mobcount] = nodePs[u];
      }
      mobcount++;
    }

  for (int u = 0; u < NUMHOST; u++)
    {
      mobcount = 0;
      Ptr<WaypointMobilityModel> waypoint =
          DynamicCast<WaypointMobilityModel> (nodes.Get (u)->GetObject<MobilityModel> ());
      for (double v = 0; v < APPSTOPSEC; v += SECINTV)
        {
          waypoint->AddWaypoint (Waypoint (Seconds (v), nodePsContainer[u][mobcount]));
          mobcount++;
        }
      std::cout << "maximum mobcount is " << mobcount << std::endl;
    }

  /*
  for (double i = 0; i < APPSTOPSEC; i += SECINTV) {
    Simulator::Schedule (Seconds(i), &testvelocity);
  }*/

  Simulator::Stop (Seconds (APPSTOPSEC));

  AnimationInterface anim ("mymobilitytest.xml");

  for (uint32_t i = 0; i < nodes.GetN (); ++i)
    {
      anim.UpdateNodeDescription (nodes.Get (i), "UAV"); // Optional
      anim.UpdateNodeColor (nodes.Get (i), 255, 0, 0); // Optional
    }

  //Config::Connect ("NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));

  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("mymobilitytest.mob"));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

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