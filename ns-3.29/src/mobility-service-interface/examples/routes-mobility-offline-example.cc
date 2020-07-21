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
#include "ns3/config-store.h"
#include "ns3/netanim-module.h"
#include "ns3/routes-mobility-helper.h"
#include "ns3/trace-helper.h"

using namespace ns3;

/**
 * This example generates mobility traces based on user downloaded XML responses from the Google Maps Directions API
 * The user needs to perform the query manually (refer to the sources, documentation of the Google Maps Directions API or to this module documentation) for more information on how to perform the request.
 * LEGAL NOTICE: The Google Maps API ToS prohibits the caching of these responses for more than 30 days.
 */

NS_LOG_COMPONENT_DEFINE ("RoutesMobilityExample");
int
main (int argc, char** argv)
{
  int numberOfNodes = 3;
  bool verbose = false;
  double centerLat = 41.1073243, centerLng = -8.6192781, centerAltitude = 0;
  std::string path = "src/mobility-service-interface/examples";
  CommandLine cmd;
  cmd.AddValue ("verbose", "add verbose logging", verbose);
  cmd.AddValue ("centerLatitude", "set the latitude for the center of the Cartesian plane", centerLat);
  cmd.AddValue ("centerLongitude", "set the longitude for the center of the Cartesian plane", centerLng);
  cmd.AddValue ("centerAltitude", "set the longitude for the center of the Cartesian plane", centerAltitude);
  cmd.AddValue ("path", "The path to the XML file or folder which contains the Google Maps Directions response.", path);
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
  //Provide a path to a folder with the XML files downloaded from the Google Maps Directions service
  routes.ChooseRoute (nodes, path);
  //As an alternative, it's also possible to specify a single file, to generate the route for a single node.
  //Uncomment, and change accordingly, the next line of code to achieve this.
  //routes.ChooseRoute(nodes.Get(0),"/path/to/XML/file");
  //Create netanim XML to visualize the output
  AnimationInterface anim ("routes-offline-animation-example.xml");
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("routes-offline-trace-example.mob"));
  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}

