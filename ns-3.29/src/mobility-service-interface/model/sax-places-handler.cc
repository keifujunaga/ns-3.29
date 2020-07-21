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
#include <iostream>
#include <xercesc/sax2/Attributes.hpp>

#include "sax-places-handler.h"
NS_LOG_COMPONENT_DEFINE ("SaxPlacesHandler");
namespace ns3 {

SaxPlacesHandler::SaxPlacesHandler (PlaceList& placeList) : placeList_ (placeList)
{
  NS_LOG_FUNCTION (this);
  m_foundResult = false;
  m_foundLat = false;
  m_foundLng = false;
  m_foundViewport = false;
  m_foundNextPageToken = false;
  m_lat = 0;
  m_lng = 0;
  m_pageToken = "";
}

void
SaxPlacesHandler::startElement (const XMLCh * const uri, const XMLCh * const localname, const XMLCh * const qname, const xercesc::Attributes& attrs)
{
  NS_LOG_FUNCTION (this);
  char* message = xercesc::XMLString::transcode (localname);
  //std::cout << "I saw element: "<< message << std::endl;
  if (strcmp (message, "result") == 0)
    {
      m_foundResult = true;
    }
  else
    {
      if (m_foundResult)
        {
          if (strcmp (message, "viewport") == 0)
            {
              m_foundViewport = true;
            }
          if (!m_foundViewport)
            {
              if (strcmp (message, "lat") == 0)
                {
                  m_foundLat = true;
                }
              else
                {
                  if (strcmp (message, "lng") == 0)
                    {
                      m_foundLng = true;
                    }
                }
            }
        }
    }
  if (strcmp (message, "next_page_token") == 0)
    {
      m_foundNextPageToken = true;
    }
  if (strcmp (message, "status") == 0)
    {
      m_foundStatus = true;
    }
  xercesc::XMLString::release (&message);
}

void
SaxPlacesHandler::characters (const XMLCh * const chars, const XMLSize_t length)
{
  std::string status = "";
  NS_LOG_FUNCTION (this);
  if (m_foundResult)
    {
      if (m_foundLat)
        {
          m_lat = atof (xercesc::XMLString::transcode (chars));
        }
      else
        {
          if (m_foundLng)
            {
              m_lng = atof (xercesc::XMLString::transcode (chars));
            }
        }
    }
  if (m_foundNextPageToken)
    {
      m_pageToken = xercesc::XMLString::transcode (chars);
    }
  if (m_foundStatus)
    {
      status = std::string (xercesc::XMLString::transcode (chars));
      if (status[0] != '\n')
        {
          NS_LOG_DEBUG ("Places Status response: " << status);
          NS_ASSERT_MSG ((strcmp (status.c_str (),"NOT_FOUND") != 0), "The location specified is not valid.");
          NS_ASSERT_MSG ((strcmp (status.c_str (),"ZERO_RESULTS") != 0), "No route could be found between the points given.");
          NS_ASSERT_MSG ((strcmp (status.c_str (),"MAX_WAYPOINTS_EXCEEDED") != 0), "You've exceeded the maximum number of waypoints. The maximum allowed is 8 waypoints");
          NS_ASSERT_MSG ((strcmp (status.c_str (),"OVER_QUERY_LIMIT") != 0), "You are above the limit for queries to the Google Maps Directions API");
          NS_ASSERT_MSG ((strcmp (status.c_str (),"REQUEST_DENIED") != 0), "The request was denied by the service!");
          NS_ASSERT_MSG ((strcmp (status.c_str (),"UNKOWN_ERROR") != 0), "A request could not be processed due to server-side errors. Please try again.");
          NS_ASSERT_MSG ((strcmp (status.c_str (),"INVALID_REQUEST") != 0), "The request was denied by the service!");
        }
    }
}

void
SaxPlacesHandler::endElement (
  const XMLCh * const uri,
  const XMLCh * const localname,
  const XMLCh * const qname
  )
{
  NS_LOG_FUNCTION (this);
  char* message = xercesc::XMLString::transcode (localname);

  if (m_foundResult)
    {
      if (strcmp (message, "viewport") == 0)
        {
          m_foundViewport = false;
        }
      if (!m_foundViewport)
        {
          if (m_foundLat)
            {
              if (strcmp (message, "lat") == 0)
                {
                  m_foundLat = false;
                }
            }
          else
            {
              if (m_foundLng)
                {
                  if (strcmp (message, "lng") == 0)
                    {
                      m_foundLng = false;
                    }
                }
            }
        }
    }
  if (m_foundResult)
    {
      if (strcmp (message, "result") == 0)
        {
          m_foundResult = false;
          ns3::Ptr<Place> place = new ns3::Place (m_lat, m_lng);
          placeList_.push_back (place);
          m_lat = 0;
          m_lng = 0;
        }
    }
  if (m_foundNextPageToken)
    {
      (*(placeList_.back ())).SetNextPageToken (m_pageToken);
      m_pageToken = "";
      m_foundNextPageToken = false;
    }
  if(m_foundStatus)
    {
    m_foundStatus=false;
    }
}

void
SaxPlacesHandler::fatalError (const xercesc::SAXParseException& exception)
{
  NS_LOG_FUNCTION (this);
  char* message = xercesc::XMLString::transcode (exception.getMessage ());
  std::cout << "Fatal Error: " << message
            << " at line: " << exception.getLineNumber ()
            << std::endl;
  xercesc::XMLString::release (&message);
}

SaxPlacesHandler::~SaxPlacesHandler ()
{
  NS_LOG_FUNCTION (this);
}

}
