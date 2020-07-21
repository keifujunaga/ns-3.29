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
#ifndef GOOGLEMAPSAPICONNECT_H
#define GOOGLEMAPSAPICONNECT_H

#include <string>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <list>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include "ns3/google-maps-decoder.h"
#include "directions-api-connect.h"
#include "leg.h"
#include "point.h"
namespace ns3 {
/**
* \ingroup mobility
*
* \brief This class is responsible for creating a connection to the Google Maps Directions service
*/
class GoogleMapsApiConnect : public DirectionsApiConnect
{
public:
  /**
   * \brief Creates a GoogleMapsApiConnect object
   *
   * Creates a GoogleMapsApiConnect object, initializing its attributes to the default values
   */
  GoogleMapsApiConnect ();
  /**
   * \brief Creates a GoogleMapsApiConnect object
   *
   * \param lat The latitude, in the WGS84 format
   * \param lng The longitude, in the WGS84 format
   * \param altitude The altitude, in meters
   */
  GoogleMapsApiConnect (double lat, double lng, double altitude);
  /**
   * \brief Creates a GoogleMapsApiConnect object
   *
   * \param apiKey The API key of the user
   * \param lat The latitude, in the WGS84 format
   * \param lng The longitude, in the WGS84 format
   * \param altitude The altitude, in meters
   */
  GoogleMapsApiConnect (std::string apiKey, double lat, double lng, double altitude);
  /**
   * \brief Creates a GoogleMapsApiConnect object
   *
   * Standard copy constructor
   * \param orig
   */
  GoogleMapsApiConnect (const GoogleMapsApiConnect& orig);
  virtual ~GoogleMapsApiConnect ();
  /**
   * \brief Performs a request to the Google Maps Directions API
   *
   * This function is responsible for performing the request to the Google Maps Directions API.
   * The API returns an XML, which is then parsed and decoded.
   *
   * \param startingPoint The street name/place/latitude-longitude pair which the user wants to set as the starting point
   * \param endPoint The street name/place/latitude-longitude pair which the user wants to set as the ending point
   * \param travelMethod The transportation mode to use when calculation the route. Valid options are: {"driving", "walking","bicycling","transit"}
   * \param departureTime The departure time for the journey
   * \param doDownload Set to true if the user requests a download. Default is false
   * \param pathToFile The path to the file to be created containing the XML information downloaded from the Google Maps Directions API
   *
   * \return m_legList A list of Legs which contain the points ready to be converted into ns3::Waypoint
   */
  std::list<Ptr<Leg> > PerformRequest (std::string startingPoint, std::string endPoint, std::string travelMethod, std::string departureTime, bool doDownload = false, std::string pathToFile = "");
  /**
   * \brief
   */
  std::list<Ptr<Leg> > PerformOfflineRequest (std::string path);
private:
  GoogleMapsDecoder m_decoder;
  std::string m_startingPoint, m_endPoint;
  std::string m_requestURL, m_apiKey;
  std::list<Ptr<Leg> > m_legList;
  curlpp::Easy m_Request;
  /**
   * \brief Get the API key.
   *
   * This function is responsible for finding and retrieving the user's Google Maps API Key
   *
   * \param path The path to the file storing the API key
   *
   * \return m_apiKey The API key
   */
  std::string GetApiKey (std::string path = "./src/mobility-service-interface/conf/api-key.txt"); 

  /**
   * \brief Parse the XML returned by the Google Maps Directions API
   *
   * This function is responsible for parsing the XML returned by the API and decode the information within.
   *
   * \param xml The XML returned by the Google Maps Directions API
   */
  int ParseXml (std::string xml, bool isOffline);
  /**
   * \brief Escapes characters not allowed in URLs from the user input
   *
	 * This function escapes all characters not allowed in URLs, as defined in
	 * RFC 3986.
	 *
	 * The function will only treat user input, as the rest of the URL is stattically defined 
   * \param input The user input to escape.
   */
  void TreatUrl (std::string &input);
  /**
   * \brief This function is responsible for saving the XML to a file on the local filesystem
   *
   * \param xml The std::string to store in the file
   * \param pathToFile The path to the file to be created in the local filesystem
   *
   */
  void SaveXmlToFile (std::string xml, std::string pathToFile);
};
}
#endif  /* GOOGLEMAPSAPICONNECT_H */

