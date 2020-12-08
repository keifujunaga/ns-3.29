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

NS_LOG_COMPONENT_DEFINE ("RoutesMobilityExample");
int
main (int argc, char** argv)
{
  int numberOfNodes = 3;
  bool verbose = false;
  double centerLat = 41.1073243, centerLng = -8.6192781, centerAltitude = 0;
  CommandLine cmd;
  cmd.AddValue ("verbose", "add verbose logging", verbose);
  cmd.AddValue ("centerLatitude", "set the latitude for the center of the Cartesian plane", centerLat);
  cmd.AddValue ("centerLongitude", "set the longitude for the center of the Cartesian plane", centerLng);
  cmd.AddValue ("centerAltitude", "set the longitude for the center of the Cartesian plane", centerAltitude);
  cmd.Parse (argc, argv);
  if (verbose)
    {
      LogComponentEnable ("GoogleMapsApiConnect", LOG_LEVEL_DEBUG);
      LogComponentEnable ("GoogleMapsDecoder", LOG_LEVEL_DEBUG);
      LogComponentEnable ("Leg", LOG_LEVEL_DEBUG);
      LogComponentEnable ("Place", LOG_LEVEL_DEBUG);
      LogComponentEnable ("Point", LOG_LEVEL_DEBUG);
      LogComponentEnable ("RoutesMobilityHelper", LOG_LEVEL_DEBUG);
      LogComponentEnable ("Step", LOG_LEVEL_DEBUG);
      LogComponentEnable ("WaypointMobilityModel", LOG_LEVEL_DEBUG);
    }

  //Create node container
  
  NodeContainer nodes;
  nodes.Create (numberOfNodes);

  //Set the mobility helper
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::WaypointMobilityModel");
  //Install mobility helper on the nodes
  mobility.Install (nodes);

  //Instantiate Routes Mobility Helper
  //Pass the constructor the coordinates to center the cartesian plane
  RoutesMobilityHelper routes (centerLat,centerLng,centerAltitude);
  //Choose route
  std::string startEndTokenList[] = {"Rua S. Tomé, Porto, Portugal","Rua Dr. Roberto Frias, Porto, Portugal","Travessa Doutor Barros, Porto, Portugal","Rua Mouzinho da Silveira, Porto, Portugal","Rua Alfredo Allen,Porto, Portugal","Rua da Alegria, Porto, Portugal"};
  //Pass it a list or array of start and endpoints
  routes.ChooseRoute (startEndTokenList,nodes);
  //For a simpler trace uncomment the following code
  //routes.ChooseRoute("Instituto Superior de Engenharia do Porto, Porto, Portugal", "Praça de Gomes Teixeira, Porto, Portugal",nodes.Get(0));

  //Create netanim XML to visualize the output
  AnimationInterface anim ("routes-animation-example.xml");
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("routes-trace-example.mob"));

  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}

