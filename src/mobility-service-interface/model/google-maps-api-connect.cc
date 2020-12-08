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
#include <xercesc/parsers/SAXParser.hpp>

#include <fstream>
#include <sstream>

#include "sax-handler.h"
#include "ns3/google-maps-api-connect.h"
NS_LOG_COMPONENT_DEFINE ("GoogleMapsApiConnect");
namespace ns3 {

GoogleMapsApiConnect::GoogleMapsApiConnect ()
{
  NS_LOG_FUNCTION (this);
  m_decoder = GoogleMapsDecoder ();
  this->m_apiKey = GetApiKey ();
}

GoogleMapsApiConnect::GoogleMapsApiConnect (std::string apiKey, double lat, double lng, double altitude)
{
  NS_LOG_FUNCTION (this);
  m_decoder = GoogleMapsDecoder (lat, lng, altitude);
  this->m_apiKey = apiKey;
}

GoogleMapsApiConnect::GoogleMapsApiConnect (double lat, double lng, double altitude)
{
  NS_LOG_FUNCTION (this);
  m_decoder = GoogleMapsDecoder (lat, lng, altitude);
  this->m_apiKey = GetApiKey ();
}

GoogleMapsApiConnect::GoogleMapsApiConnect (const GoogleMapsApiConnect& orig)
{
  this->m_apiKey = orig.m_apiKey;
  this->m_decoder = orig.m_decoder;
  this->m_endPoint = orig.m_endPoint;
  this->m_requestURL = orig.m_requestURL;
  this->m_startingPoint = orig.m_startingPoint;
  this->m_legList = orig.m_legList;
}

GoogleMapsApiConnect::~GoogleMapsApiConnect ()
{
  NS_LOG_FUNCTION (this);
}

void
GoogleMapsApiConnect::TreatUrl (std::string &input)
{
  NS_LOG_FUNCTION (this);
  size_t size = input.size ();
  CURL *curl = curl_easy_init ();
  if (curl)
    {
      char *output = curl_easy_escape (curl,input.c_str (),size);
      input = std::string (output);
      curl_free (output);
    }
}

std::string
GoogleMapsApiConnect::GetApiKey (std::string path)
{
  NS_LOG_FUNCTION (this);
  std::ifstream keyfile (path.c_str ());
  std::string buff = "";
  keyfile >> buff;
  if (buff == "")
    {
      NS_FATAL_ERROR ("Fatal error:  could not read API key from path " << path);
    }
  return buff;
}

std::list<Ptr<Leg> >
GoogleMapsApiConnect::PerformOfflineRequest (std::string path)
{
  NS_LOG_FUNCTION (this);
  if (!m_legList.empty ())
    {
      m_legList.clear ();
    }
  ParseXml (path, true);
  return m_legList;

}

std::list<Ptr<Leg> >
GoogleMapsApiConnect::PerformRequest (std::string startingPoint, std::string endPoint, std::string travelMethod, std::string departureTime, bool doDownload, std::string pathToFile)
{
  NS_LOG_FUNCTION (this);
  if (!m_legList.empty ())
    {
      m_legList.clear ();
    }
  try
    {
      TreatUrl (startingPoint);
      TreatUrl (endPoint);
      m_requestURL = "https://maps.googleapis.com/maps/api/directions/xml?origin=" + startingPoint + "&destination=" + endPoint + "&sensor=false" + "&mode=" + travelMethod + "&departure_time=" + departureTime + "&key=" + m_apiKey;
      std::ostringstream os;
      m_Request.setOpt<curlpp::options::Url> (m_requestURL);
      curlpp::options::WriteStream ws (&os);
      m_Request.setOpt (ws);
      m_Request.perform ();
      NS_LOG_DEBUG ("Request performed with the following URL: " << m_requestURL);
      if (doDownload)
        {
          SaveXmlToFile (os.str (), pathToFile);
          //return m_legList;
        }
      ParseXml (os.str (), false);
      sleep (2);
      return m_legList;
    }
  catch (curlpp::RuntimeError &e)
    {
      NS_FATAL_ERROR ("There was an error while contacting the Google Maps API." << std::endl << e.what ());
      return m_legList;
    }
}

int
GoogleMapsApiConnect::ParseXml (std::string xml, bool isOffline)
{
  NS_LOG_FUNCTION (this);
  StepList stepList, redirectRoute;
  Point p;
  Step step;
  std::list<Ptr<Point> > pointList;
  try
    {
      xercesc::XMLPlatformUtils::Initialize ();
    }
  catch (const xercesc::XMLException& toCatch)
    {
      char* message = xercesc::XMLString::transcode (toCatch.getMessage ());
      NS_FATAL_ERROR ("An error occured during the parser initialization!" << std::endl << "The exception message is:" << std::endl << message);
      xercesc::XMLString::release (&message);
      return 1;
    }

  xercesc::SAX2XMLReader* parser = xercesc::XMLReaderFactory::createXMLReader ();
  parser->setFeature (xercesc::XMLUni::fgSAX2CoreValidation, true);
  parser->setFeature (xercesc::XMLUni::fgSAX2CoreNameSpaces, true);   // optional
  xercesc::ContentHandler* h = new SaxHandler (m_legList);
  parser->setContentHandler (h);
  XMLByte* xmlByte = static_cast<XMLByte*> ((unsigned char*)xml.c_str ());
  MemBufInputSource myxml_buf (xmlByte, xml.length (), "dummy", false);
  try
    {
      if (isOffline)
        {
          parser->parse (xml.c_str ()); //if it's an offline request, then XML is the path for the xml file to read!
        }
      else
        {
          parser->parse (myxml_buf);
        }
      m_decoder.ConvertToCartesian (m_legList);
    }
  catch (const xercesc::XMLException& toCatch)
    {
      char* message = xercesc::XMLString::transcode (toCatch.getMessage ());
      NS_FATAL_ERROR ("An error with the parser has occurred." << std::endl << "The exception message is:" << std::endl << message);
      xercesc::XMLString::release (&message);
      return -1;
    }
  catch (const xercesc::SAXParseException& toCatch)
    {
      char* message = xercesc::XMLString::transcode (toCatch.getMessage ());
      NS_FATAL_ERROR ("An error with the parser has occurred." << std::endl << "The exception message is:" << std::endl << message);
      xercesc::XMLString::release (&message);
      return -1;
    }
  catch (...)
    {
      NS_FATAL_ERROR ("An unexpected exception was thrown!");
      return -1;
    }
  delete h;
  return 0;
}
void
GoogleMapsApiConnect::SaveXmlToFile (std::string xml, std::string pathToFile)
{
  NS_LOG_FUNCTION (this);
  std::fstream fich;
  fich.open (pathToFile.c_str (),std::ios::out);
  fich << xml;
  fich.close ();
}
}
