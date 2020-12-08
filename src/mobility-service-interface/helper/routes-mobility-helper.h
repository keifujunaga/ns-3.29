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
 * Author: Michele Albano <mialb@isep.ipp.pt> <michele.albano@gmail.com@gmail.com>
 */

#ifndef ROUTESMOBILITYHELPER_H
#define ROUTESMOBILITYHELPER_H
#include <ctime>
#include <string>
#include <deque>
#include <dirent.h>
#include <ns3/node.h>
#include <ns3/node-container.h>
#include <ns3/waypoint-mobility-model.h>
#include "directions-api-connect.h"
#include "google-maps-api-connect.h"
#include "ns3/google-maps-places-api-connect.h"
#include "ns3/place.h"

namespace ns3 {
/**
 * \ingroup mobility
 * \brief The helper class that provides access to the functionalities in this module
 *
 * This class is responsible for exposing to the user a simple and easy to use API, with which the user can create complex mobility traces
 * The member functions of this class are intended to be called from within the ns-3 simulation environment.
 */
class RoutesMobilityHelper
{
public:
  /**
   * \brief Creates a RoutesMobilityHelper object
   *
   * Creates the object with default values
   */
  RoutesMobilityHelper ();
  /**
   * \brief Creates a RoutesMobilityHelper object
   *
   * The values passed to the constructor set the center of the Cartesian plane relative to a real-world point.
   * \param lat The latitude of the real-world location
   * \param lng The longitude of the real-world location
   * \param altitude The altitude of the real-world location
   */
  RoutesMobilityHelper (double lat, double lng, double altitude);
  /**
   * \brief Creates a RoutesMobilitHelper object
   *
   * Standard copy constructor
   * \param orig
   */
  RoutesMobilityHelper (const RoutesMobilityHelper& orig);
  virtual ~RoutesMobilityHelper ();
  /**
   * \brief Generates a mobility trace from a real-world route.
   *
   * This function will generate a mobility trace from a real-world route.
   * The user is required to pass the function a street name/place/coordinate as start and endpoints, and the module will generate a mobility trace of the journey.
   * \param startPoint The start point of the journey
   * \param endPoint The end point of the journey
   * \param node The node in which to install this mobility trace
   * \return Returns 0 if successful
   */
  int ChooseRoute (std::string startPoint, std::string endPoint, Ptr<Node> node); //overload as necessary
  /**
   * \brief Sets the transportation mode that will be used to create the mobility trace
   *
   * This function allows the user to choose the transportation method that the API will use. This value will probably differ from API to API, please refer to the API you are using for more information.
   * For the Google Maps Directions API, the valid options are: {"driving", "walking","bicycling","transit"}
   *
   * \param travelMode The method of transportation
   */
  void SetTransportationMethod (std::string travelMode);
  /**
   * \brief Sets the desired departure time for the journey
   *
   * This function allows the user to set at which time the journey will start. This value is required by the Google Maps Directions service in order to produce the best route with transit information.
   *
   * \param departureTime The time that will serve as the starting time for the journey. This is a real-world time, and is used only to produce the transit information. The value specified must be in Epoch (the time, in seconds, since January 1st, 1970. The linux command date +%s will output the current time in Epoch).By default, this value is the current system time.
   */
  void SetDepartureTime (std::string departureTime);
  /**
   * \brief Generates several mobility traces from real-world routes
   *
   * This function receives an array of strings with start and endpoint tokens. The user must create an array of strings and add the start and endpoints in succession.
   * This enables the user to create mobility traces for an entire node container.
   * \param listTokenStartEndPoint The array of start and endpoint tokens.
   * \param nodeContainer The node container in which to install the generated mobility traces
   * \return Returns 0 if successful
   */
  int ChooseRoute (std::string const *listTokenStartEndPoint, NodeContainer &nodeContainer);
  /**
 * \brief Automatically generates mobility traces for an entire node container, from real-world routes
 *
 * This function uses the Google Maps Places service to download the locations of up to 60 places. These locations are then randomly selected, in order to generate routes.
 * The latitude/longitude parameters indicate the point from where the user wants to center the search.
 * \param nodeContainer The node container in which to install the generated mobility traces
 * \param lat The latitude of the center of the search
 * \param lng The longitude of the center of the search
 * \param radius The radius of the search, in meters, from the center of the search
 * \return Returns 0 if successful
 */
  int ChooseRoute (NodeContainer &nodeContainer, double lat, double lng, double radius);

  /**
   * \brief Generates a mobility trace for a node using a local file
   *
   * This function generates a mobility trace using a local file. The user needs to query the Directions service manually and download the XML file to the local filesystem
   * \param node The node in which to install this mobility trace
   * \param path The path to the local XML file
   * \return Returns 0 if successful
   */
  int ChooseRoute (Ptr<Node> node, std::string path);
  /**
   * \brief Generates a mobility trace for several nodes using local files
   *
   * This function searches a user-provided directory to find the XML files obtained from a manual query to the Directipns service.
   * Note: Currently, this function considers all files under the specified directory to be valid input!
   * \param nodeContainer The node container in which to install the generated mobility traces
   * \param dirPath The path to the directory
   * \return Returns 0 if successful
   */
  int ChooseRoute (NodeContainer &nodeContainer, std::string dirPath);
  /**
   * \brief Generates a mobility trace for a node, redirecting it when required by the user
   *
   * This function will recalculate a route at a specified time, effectively redirecting the node, from its current position, to the new destination.
   *
   * Note: This function will not work offline
   * \param node The node in which to install the mobility trace
   * \param start The original starting point
   * \param end The original destination point
   * \param redirectedDestination The destination point after the redirect operation
   * \param timeToTrigger The time, in seconds, to trigger the redirect
         *
         * \return Returns 0 if sucessful
   */
  int ChooseRoute (Ptr<Node> node, std::string start, std::string end, std::string redirectedDestination, double timeToTrigger); //FUTURE: This function will probably need code cleanup in the future!

  /**
  * This function uses the Google Maps Places service to download the locations of up to 60 places. These locations are then used to, randomly,  generate routes.
  * The latitude/longitude parameters indicate the point from where the user wants to center the search. In this case the user needs to specify two search areas, in order to have the nodes move between two real world areas.

  * \param nodeContainer The node container in which to install the generated mobility traces
  * \param lat The latitude of the center of the search, for the start area
  * \param lng The longitude of the center of the search, for the start area
  * \param radius The radius of the search, in meters, from the center of the search, for the start area
  * \param destLat The latitude of the center of the search, for the start area
  * \param destLng The longitude of the center of the search, for the start area
  * \param destRadius The radius of the search, in meters, from the center of the search, for the start area
  * \return Returns 0 if successful
  */
  int ChooseRoute (NodeContainer &nodeContainer, double lat, double lng, double radius, double destLat, double destLng, double destRadius);

  /**
   * \brief Automatically generates mobility traces for an entire node container within the user specified area.
   *
   * This function works by randomly picking coordinates that are within the user specified area, as start and endpoints.
   * The user needs to pass the function the maximum and minimum values of latitude and longitude.
   *
         * \param nodes The node container in which to install the mobility traces
   * \param upperLat The real world maximum latitude for the area
   * \param upperLng The real world maximum longitude for the area
   * \param lowerLat The real world minimum latitude for the area
   * \param lowerLng The real world minimum longitude for the area
   * \return Returns 0 if successful
   */
  int ChooseRoute (NodeContainer& nodes, double upperLat, double upperLng, double lowerLat, double lowerLng);

  /**
   * \brief Generates mobility traces for an entire node container, with the nodes moving between two user specified real world locations
   *
   * This function works by selecting start points from a user specified area and destination points from a different user specified area.
   * This function is useful when the user wants the nodes to move between two different real world areas.
   *
   * The user is required to pass the function the maximum and minimum values of latitude and longitude for both areas.
   *
         * \param nodes The node container in which to install the mobility traces
   * \param startUpperLat The real world maximum latitude for the start area
   * \param startUpperLng The real world maximum longitude for the start area
   * \param startLowerLat The real world minimum latitude for the start area
   * \param startLowerLng The real world minimum longitude for the start area
   * \param destUpperLat The real world maximum latitude for the destination area
   * \param destUpperLng The real world maximum longitude for the destination area
   * \param destLowerLat The real world minimum latitude for the destination area
   * \param destLowerLng The real world minimum longitude for the destination area
   * \return Returns 0 if successful
   */
  int ChooseRoute (NodeContainer& nodes, double startUpperLat, double startUpperLng, double startLowerLat, double startLowerLng,
                   double destUpperLat, double destUpperLng, double destLowerLat, double destLowerLng);
private:
  DirectionsApiConnect *m_strategy; /*!< The strategy interface to use */
  std::string m_travelMode; /*!< The travel method selected */
  std::string m_departureTime; /*!< The real world departure time for the nodes, in EPOCH. By default, the current clock time of the system will be selected */
  /**
   * \brief Add the Points to the WaypointMobilityModel
   *
   * This function takes all the Points of the std::list<Ptr<Leg> > and passes them to the WaypointMobilityModel.
   * It enables the creation of ns3::Waypoints for every Point in the journey (which might include several legs!)
   * \param node The node in which to schedule the Points
   * \param legList The LegList with all the Points
   *
   * \return The number of ns3::Waypoints added to the WaypointMobilityModel of that node
   */
  int SchedulePoints (Ptr<Node> node,std::list<Ptr<Leg> > legList);
  /**
   * \brief Generate a random value inside the specified parameters
         *
   * This function will generate a random value that is lower than the maximum value specified and higher than the minimum value specified.
   * The function takes two arguments, the minimum value and the maximum value, respectively.
         *
         * Note: This function should be replaced, in the future, by the RNGs implemented in ns3
         * \param min The minimum value of the random number
   * \param max the maximum value of the random number
   *
   * \return The random number generated
   */
  double GenerateRandomValue (double min, double max);
  /**
   * \brief Opens a directory and reads the filenames of all the XML files in that folder.
         *
         * This function is used to open a directory and read the filenames of all the XML files present in that folder.
         * The function then returns a std::list containing all the filenames.
         *
         * \param dirPath The path to the directory to read from
   *
   * \return A list containing all the filenames found in the directory.
   */
  std::list<std::string> Open (std::string dirPath);
  /**
   * \brief Converts a latitude/longitude set into a string. It takes the latitude and longitude values and concatenate them into a single string.
         * For example, the latitude value of 41.36 and the longitude value of -8.92 will translate into the string "41.36,-8.92"
         *
         * This function is particularly useful when using latitude and longitude values with the ChooseRoute(std::string,std::string,Ptr<Node>)
         *
         * \param lat The latitude value
         * \param lng The longitude value
   *
   * \return The converted string
   */
  std::string LocationToString (double lat, double lng);
  /**
   * \brief Gets the local clock time of the system
         *
         * This function will return a string containing the EPOCH time of the system.
         *
   * \return The current time
   */
  std::string GetCurrentTime ();
};
}
#endif  /* ROUTESMOBILITYHELPER_H */

