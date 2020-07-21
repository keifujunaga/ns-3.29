
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

#ifndef ISTRATEGYAPICONNECT_H
#define ISTRATEGYAPICONNECT_H

#include <string>
#include "ns3/leg.h"
namespace ns3 {
/**
 * \ingroup mobility
 * \brief An interface the API used to request information to be used by the module
 *
 * The Strategy design pattern is used in order to allow the integration of different mapping and route planning APIs with ns-3
 */
class DirectionsApiConnect
{
public:
  /**
   * \brief Performs a request to the Google Maps Directions service
   *
   * This function is abstract, and must be implemented
   * \param startingPoint The street name/place/latitude-longitude pair which the user wants to set as the starting point
   * \param endPoint The street name/place/latitude-longitude pair which the user wants to set as the ending point
         * \param travelMethod The transportation mode to use when calculation the route. Valid options are: {"driving", "walking","bicycling","transit"}
         * \param departureTime The departure time for the journey
         * \param doDownload Set to true if the user requests a download. Default is false
         * \param pathToFile The path to the file to be created containing the XML information downloaded from the Google Maps Directions API
         *
         * \return m_legList A list of Legs which contain the points ready to be converted into ns3::Waypoint
         */
  virtual std::list<Ptr<Leg> > PerformRequest (std::string startingPoint, std::string endPoint, std::string travelMethod, std::string departureTime, bool doDownload = false, std::string pathToFile = "") = 0;
  /**
  * \brief Performs an offline "request"
  *
  * This function reads the input otherwise provided by Google Maps from a local file
  * This function is abstract, and must be implemented
        *
        * \param path The path to the API's response file
        * \return m_legList A list of Legs which contain the points ready to be converted into ns3::Waypoint
  */
  virtual std::list<Ptr<Leg> > PerformOfflineRequest (std::string path) = 0;   //overload as necessary
private:
};
}
#endif  /* ISTRATEGYAPICONNECT_H */

