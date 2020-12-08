/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 CISTER - Research Center in Real-Time & Embedded Computing Systems
 *
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
 *
 * Author: Tiago Cerqueira <1090678@isep.ipp.pt> <tiago.miguel43@gmail.com>
 */

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/routes-mobility-helper.h"
#include "ns3/config-store.h"
#include "ns3/trace-helper.h"

using namespace ns3;

/**
 * This example queries the Google Maps Directions API to generate a mobility trace based on a real-world route.
 * The mobility traces generated in this example are from the city of Porto, Portugal.
 */

NS_LOG_COMPONENT_DEFINE ("RandomWaypointMobilityExample");
int
main (int argc, char** argv)
{
  int randnum = 6;
  int numberOfNodes = 3;
  //double nodeSpeed = 5.56; //speed no henkou
  //double nodeSpeed = 1.23;
  double appDataStart = 0.0;
  double appDataEnd = 3000.0;
  double appTotalTime = appDataStart + appDataEnd; // simulator stop time

  //Create node container
  
  NodeContainer mobilenode;
  mobilenode.Create (numberOfNodes);

  //Set the mobility helper
  MobilityHelper mobility1;
  MobilityHelper mobility2;
  MobilityHelper mobility3;

  std::srand (randnum);

  Ptr<ListPositionAllocator> initialAlloc1 = CreateObject<ListPositionAllocator> ();
  initialAlloc1->Add (Vector (200, 1300, 1.5));
  Ptr<ListPositionAllocator> initialAlloc2 = CreateObject<ListPositionAllocator> ();
  initialAlloc2->Add (Vector (1500, 500, 1.5));
  Ptr<ListPositionAllocator> initialAlloc3 = CreateObject<ListPositionAllocator> ();
  initialAlloc3->Add (Vector (2500, 1500, 1.5));

  mobility1.SetPositionAllocator (initialAlloc1);
  mobility2.SetPositionAllocator (initialAlloc2);
  mobility3.SetPositionAllocator (initialAlloc3);

  /*
  mobility1.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX",
                              DoubleValue (1000), "MaxX", DoubleValue (2000), "MinPause",
                              DoubleValue (0), "MaxPause", DoubleValue (100), "MinY",
                              DoubleValue (0), "MaxY", DoubleValue (1000), "Z", DoubleValue (1.5));
  */
  mobility1.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", 
                              "Bounds", StringValue("0|1000|1000|2000"),
                              "Speed", StringValue("ns3::UniformRandomVariable[Min=10|Max=10]"),
                              "Distance", DoubleValue(100));
  mobility1.Install (mobilenode.Get (0));

  /*
  mobility2.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX",
                              DoubleValue (0), "MaxX", DoubleValue (1000), "MinPause",
                              DoubleValue (0), "MaxPause", DoubleValue (100), "MinY",
                              DoubleValue (1000), "MaxY", DoubleValue (2000), "Z", DoubleValue (1.5));
  */
  mobility2.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", 
                              "Bounds", StringValue("1000|2000|0|1000"),
                              "Speed", StringValue("ns3::UniformRandomVariable[Min=10|Max=10]"),
                              "Distance", DoubleValue(100));
  mobility2.Install (mobilenode.Get (1));

  /*
  mobility3.SetMobilityModel ("ns3::SteadyStateRandomWaypointMobilityModel", "MinSpeed",
                              DoubleValue (nodeSpeed), "MaxSpeed", DoubleValue (nodeSpeed), "MinX",
                              DoubleValue (2000), "MaxX", DoubleValue (3000), "MinPause",
                              DoubleValue (0), "MaxPause", DoubleValue (100), "MinY",
                              DoubleValue (1000), "MaxY", DoubleValue (2000), "Z", DoubleValue (1.5));
  */

  mobility3.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", 
                              "Bounds", StringValue("2000|3000|1000|2000"),
                              "Speed", StringValue("ns3::UniformRandomVariable[Min=10|Max=10]"),
                              "Distance", DoubleValue(100));
  mobility3.Install (mobilenode.Get (2));

  //Create netanim XML to visualize the output
  AnimationInterface anim ("random-waypoint-mobility.xml");
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("random-waypoint-mobility-2-6.mob"));

  Simulator::Stop (Seconds (appTotalTime));
  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}

