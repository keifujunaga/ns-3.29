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

#ifndef GOOGLEMAPSPLACESAPICONNECT_H
#define GOOGLEMAPSPLACESAPICONNECT_H

#include <list>

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include <ns3/place.h>
#include "sax-places-handler.h"
#include "places-api-connect.h"

namespace ns3 {
/**
 * \ingroup mobility
 *
 * \brief This class is responsible for creating the connection to the Google Maps Places service
 */
class GoogleMapsPlacesApiConnect : public PlacesApiConnect
{
public:
  /**
   * \brief Creates a GoogleMapsPlacesApiConnect object
   *
   * This constructor creates the object, initializing its attributes to the default values
   */
  GoogleMapsPlacesApiConnect ();
  /**
   * \brief Creates a GoogleMapsPlacesApiConnect object
   *
   * \param path The path to the file containing the API information
   */
  GoogleMapsPlacesApiConnect (std::string path);
  /**
   * \brief Creates a GoogleMapsPlacesApiConnect object
   *
   * Standard copy constructor
   *
   * \param orig The object to copy
   */
  GoogleMapsPlacesApiConnect (const GoogleMapsPlacesApiConnect& orig);
  virtual ~GoogleMapsPlacesApiConnect ();
  /**
   * \brief Performs a request to the Google Maps Places service
   *
   * This function is responsible for performing the request to the Google Maps Places service.
   * The API returns an XML, which is then parsed for its contents.
   *
   * \param lat The latitude of the center of the search
   * \param lng The longitude of the center of the search
   * \param radius The radius of the search, in meters.
   *
   * \return m_placeList A std::list of Places
   */
  std::vector<Ptr<Place> > PerformRequest (double lat, double lng, double radius);
private:
  /**
   * \brief Transforms a double into a std::string
   *
   * \param number The double to transform
   */
  std::string DoubleToString (double number);
  /**
   * \brief Get the API key.
   *
   * This funtion is responsible for finding and retrieving the user's Google Maps API Key
   *
   * \param path The path to the file storing the API key
   *
   * \return m_apiKey The API key
   */
  std::string GetApiKey (std::string path = "");
  /**
   * \brief Parse the XML returned by the Google Maps Places service
   *
   * This function is responsible for parsing the XML returned by the API.
   *
   * \param xml The XML returned by the Google Maps Places service
   */
  int ParseXml (std::string xml);

  std::string m_apiKey;
  PlaceList m_placeList;
  curlpp::Easy m_request;
  std::string m_requestUrl;
};

}
#endif  /* GOOGLEMAPSPLACESAPICONNECT_H */

