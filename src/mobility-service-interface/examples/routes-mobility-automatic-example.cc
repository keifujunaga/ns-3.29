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
#include "ns3/routes-mobility-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"
#include "ns3/netanim-module.h"
#include "ns3/trace-helper.h"

using namespace ns3;

/**
 * This example illustrates the use of this module when generating mobility automatically for ns3::NodeContainers 
 *
 * The automatic trace generation follows two distinct approaches. The user
 * is able to specify an area in which the module should, randomly, choose
 * coordinates to use as start and endpoints. These coordinates will be 
 * inside the area specified by the user.
 *
 * In alternative, mobility generation can also be performed by querying the Google Maps Places API for places (restaurants, cinemas, etc), which the module will then use as start and endpoints.
 *
 * In this example, both approaches are illustrated. For the first method, 
 * an area is selected, in Downtown Porto, Portugal. For the second method,
 *  a set of coordinates is chosen to be the center of the search for places.
 *  These coordinates represent a real-world location, in this case, somewhere
 *  in Porto, Portugal.
 */

NS_LOG_COMPONENT_DEFINE ("RoutesMobilityExample");
int
main (int argc, char** argv)
{
  int numberOfNodes = 100; 
  bool verbose = false;
  double centerLat = 41.1073243, centerLng = -8.6192781, centerAltitude = 0, searchLat = 41.171770, searchLng = -8.611038, searchRadius = 5000;
  CommandLine cmd;
  cmd.AddValue ("verbose", "add verbose logging", verbose);
  cmd.AddValue ("centerLatitude", "set the latitude for the center of the Cartesian plane", centerLat);
  cmd.AddValue ("centerLongitude", "set the longitude for the center of the Cartesian plane", centerLng);
  cmd.AddValue ("centerAltitude", "set the longitude for the center of the Cartesian plane", centerAltitude);
  cmd.AddValue ("searchLat", "The real-world latitude coordinate for the center of the search for Google Maps Places", searchLat);
  cmd.AddValue ("searchLng", "The real-world longitude coordinate for the center of the search for Google Maps Places", searchLng);
  cmd.AddValue ("searchRadius", "The real-world search radius, in meters, from the center of the search. This value must be between 0 to 5000 meters", searchRadius);
  cmd.Parse (argc, argv);
  if (verbose)
    {
      LogComponentEnable ("GoogleMapsApiConnect", LOG_LEVEL_DEBUG);
      LogComponentEnable ("GoogleMapsDecoder", LOG_LEVEL_DEBUG);
      LogComponentEnable ("GoogleMapsPlacesApiConnect", LOG_LEVEL_ALL);
      LogComponentEnable ("SaxPlacesHandler", LOG_LEVEL_DEBUG);
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
	//To quickly generate a mobility trace with locations within a specified area the following code can be used.
	//This will pick coordinates, at random, inside the user specified area.
  routes.ChooseRoute (nodes, 41.161093, -8.633352, 41.146487, -8.606916);
	//As an alternative, the user can produce a similar result by using coordiantes retrieved from the Google Maps Places API
  //The module queries the Google Maps Places service to find start and endpoints it can use.
  //This function takes the node container, the latitude and longitude of the center of the search and the search's radius
  //routes.ChooseRoute (nodes, searchLat, searchLng,searchRadius);
  //Create netanim XML to visualize the output
  AnimationInterface anim ("routes-automatic-animation-example.xml");
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("routes-automatic-trace-example.mob"));
  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}

