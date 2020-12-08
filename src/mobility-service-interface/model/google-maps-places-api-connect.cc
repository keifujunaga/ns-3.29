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
#include "ns3/google-maps-places-api-connect.h"
#include <fstream>
#include <sstream>

NS_LOG_COMPONENT_DEFINE ("GoogleMapsPlacesApiConnect");

namespace ns3 {

  GoogleMapsPlacesApiConnect::GoogleMapsPlacesApiConnect ()
    {
    NS_LOG_FUNCTION (this);
    m_apiKey = GetApiKey ();
    }

  GoogleMapsPlacesApiConnect::GoogleMapsPlacesApiConnect (std::string path)
    {
    NS_LOG_FUNCTION (this);
    this->m_apiKey = GetApiKey (path);
    }

  GoogleMapsPlacesApiConnect::GoogleMapsPlacesApiConnect (const GoogleMapsPlacesApiConnect& orig)
    {
    this->m_apiKey = orig.m_apiKey;
    this->m_placeList = orig.m_placeList;
    }

  GoogleMapsPlacesApiConnect::~GoogleMapsPlacesApiConnect ()
    {
    NS_LOG_FUNCTION (this);
    }

  std::vector<Ptr<Place> >
  GoogleMapsPlacesApiConnect::PerformRequest (double lat, double lng, double radius)
    {
    NS_LOG_FUNCTION (this);
    NS_ASSERT_MSG ((radius <= 5000 && radius > 0), "The radius value must be from 0 to 5000 meters.");
    std::string s_lat = "", s_lng = "", s_radius = "";
    s_lat = DoubleToString (lat);
    s_lng = DoubleToString (lng);
    s_radius = DoubleToString (radius);
    int i = 0;
    Ptr<Place> lastPlace = new Place();
    try
      {
      do
        {
        if (m_placeList.size () == 0)
          {
          m_requestUrl = "https://maps.googleapis.com/maps/api/place/nearbysearch/xml?location=" + s_lat + "," + s_lng + "&radius=" + s_radius + "&key=" + m_apiKey;
          }
        else
          {
          m_requestUrl = "https://maps.googleapis.com/maps/api/place/nearbysearch/xml?location=" + s_lat + "," + s_lng + "&radius=" + s_radius + "&pagetoken=" + lastPlace->GetNextPageToken () + "&key=" + m_apiKey;
          lastPlace = new Place();
          }
	NS_LOG_DEBUG("Request performed with the URL: " << m_requestUrl);
        std::ostringstream os;
        m_request.setOpt<curlpp::options::Url>(m_requestUrl);
        curlpp::options::WriteStream ws (&os);
        m_request.setOpt (ws);
        m_request.perform ();
        ParseXml (os.str ());
        lastPlace = new Place ((*m_placeList.back ()));
        i++;
        m_request.reset ();
        sleep(2);
        }
      while (lastPlace->GetNextPageToken () != "");
      if(g_log.IsEnabled(LOG_DEBUG))
          {
        NS_LOG_DEBUG("Iterating through the list of Places:");
        for(std::vector<Ptr<Place> >::iterator j = m_placeList.begin();j!=m_placeList.end();j++)
          {
          NS_LOG_DEBUG(*(*j));
          }
          }
      return m_placeList;
      }
    catch (curlpp::RuntimeError &e)
      {
      NS_FATAL_ERROR ("There was an error while contacting the Google Maps API." << std::endl << e.what ());
      return m_placeList;
      }
    }

  std::string
  GoogleMapsPlacesApiConnect::GetApiKey (std::string path)
    {
    NS_LOG_FUNCTION (this);
    if(path == "")
     {
	path = "src/mobility-service-interface/conf/api-key.txt";
     }
    std::ifstream keyfile (path.c_str());
    std::string buff;
    keyfile >> buff;
    if (buff == "")
     {
     NS_FATAL_ERROR ("Fatal error:  could not read API key from path " << path);
     }
    return buff;
    }

  int
  GoogleMapsPlacesApiConnect::ParseXml (std::string xml)
    {
    NS_LOG_FUNCTION (this);
    try
      {
      xercesc::XMLPlatformUtils::Initialize ();
      }
    catch (const xercesc::XMLException& toCatch)
      {
      char* message = xercesc::XMLString::transcode (toCatch.getMessage ());
      NS_LOG_ERROR ("An error with the parser has occurred." << std::endl << "The exception message is:" << std::endl << message);
      xercesc::XMLString::release (&message);
      return 1;
      }

    xercesc::SAX2XMLReader* parser = xercesc::XMLReaderFactory::createXMLReader ();
    parser->setFeature (xercesc::XMLUni::fgSAX2CoreValidation, true);
    parser->setFeature (xercesc::XMLUni::fgSAX2CoreNameSpaces, true); // optional
    xercesc::ContentHandler* h = new SaxPlacesHandler (m_placeList);
    parser->setContentHandler (h);
    //parser->setErrorHandler(h);

    XMLByte* xmlByte = static_cast<XMLByte*> ((unsigned char*)xml.c_str());
    MemBufInputSource myxml_buf (xmlByte, xml.length (), "dummy", false);

    try
      {
      parser->parse (myxml_buf);
      }
    catch (const xercesc::XMLException& toCatch)
      {
      char* message = xercesc::XMLString::transcode (toCatch.getMessage ());
      NS_LOG_ERROR ("An error with the parser has occurred." << std::endl << "The exception message is:" << std::endl << message);
      xercesc::XMLString::release (&message);
      return -1;
      }
    catch (const xercesc::SAXParseException& toCatch)
      {
      char* message = xercesc::XMLString::transcode (toCatch.getMessage ());
      NS_LOG_ERROR ("An error with the parser has occurred." << std::endl << "The exception message is:" << std::endl << message);
      xercesc::XMLString::release (&message);
      return -1;
      }
    catch (...)
      {
      NS_LOG_ERROR ("An unexpected exception was thrown!");
      return -1;
      }
    delete h;
    return 0;
    }

  std::string
  GoogleMapsPlacesApiConnect::DoubleToString (double number)
    {
    NS_LOG_FUNCTION (this);
    std::ostringstream strs;
    strs << number;
    return strs.str ();
    }
}//namespace ns3
