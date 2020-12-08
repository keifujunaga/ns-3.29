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

#include "sax-handler.h"
NS_LOG_COMPONENT_DEFINE ("SaxHandler");
namespace ns3 {

SaxHandler::SaxHandler (LegList& legList) : legList_ (legList)
{
  NS_LOG_FUNCTION (this);
  foundDurationValue = false;
  foundDuration = false;
  foundLeg = false;
  foundPolyline = false;
  foundStep = false;
  foundStatus = false;
  durationOfStep = 0;
  polyline = "";
  status = "";
}

void
SaxHandler::startElement (const XMLCh * const uri, const XMLCh * const localname, const XMLCh * const qname, const xercesc::Attributes& attrs)
{
  NS_LOG_FUNCTION (this);
  char* message = xercesc::XMLString::transcode (localname);
  //std::cout << "I saw element: "<< message << std::endl;
  if (strcmp (message, "leg") == 0)
    {
      foundLeg = true;
    }
  else
    {
      if (foundStep)
        {
          if (strcmp (message, "value") == 0)
            {
              if (foundDuration)
                {
                  foundDurationValue = true;
                }
            }
          else
            {
              if (strcmp (message, "points") == 0)
                {
                  foundPolyline = true;
                }
            }
        }
    }
  if (strcmp (message, "duration") == 0)
    {
      foundDuration = true;
    }
  if (strcmp (message, "step") == 0)
    {
      foundStep = true;
    }
  if (strcmp(message, "status") == 0)
    {
    foundStatus = true;
    }
  xercesc::XMLString::release (&message);
}

void
SaxHandler::characters (const XMLCh * const chars, const XMLSize_t length)
{
  NS_LOG_FUNCTION (this);
  if (foundPolyline)
    {
      std::string str = "";
      str = std::string (xercesc::XMLString::transcode (chars));
      if (str[0] != '\n')
        {
          polyline = std::string (str);
        }
    }
  else
    {
      if (foundDurationValue)
        {
          durationOfStep = atoi ((xercesc::XMLString::transcode (chars)));
        }
    }
  if (foundStatus)
    {
      status = std::string (xercesc::XMLString::transcode (chars));
      if (status[0] != '\n')
        {
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
SaxHandler::endElement (
  const XMLCh * const uri,
  const XMLCh * const localname,
  const XMLCh * const qname
  )
{
  NS_LOG_FUNCTION (this);
  char* message = xercesc::XMLString::transcode (localname);
  if (foundStatus)
    {
      foundStatus = false;
    }
  if (strcmp (message, "leg") == 0)
    {
      foundLeg = false;
    }
  else
    {
      if (foundStep)
        {
          if (strcmp (message, "value") == 0)
            {
              if (foundDuration)
                {
                  foundDuration = false;
                  foundDurationValue = false;
                }
            }
          else
            {
              if (strcmp (message, "polyline") == 0)
                {
                  foundPolyline = false;
                }
            }
        }

    }
  if (strcmp (message, "step") == 0)
    {
      foundStep = false;
      if (polyline != "\n ")
        {
          ns3::Ptr<ns3::Step> s = new ns3::Step (polyline, durationOfStep);
          stepList_.push_back (s);
        }
      polyline = "";
      durationOfStep = 0;
    }
  if (strcmp (message, "leg") == 0)
    {
      ns3::Ptr<ns3::Leg> l = new ns3::Leg (stepList_);
      legList_.push_back (l);
      stepList_.clear ();
    }
}

void
SaxHandler::fatalError (const xercesc::SAXParseException& exception)
{
  NS_LOG_FUNCTION (this);
  char* message = xercesc::XMLString::transcode (exception.getMessage ());
  std::cout << "Fatal Error: " << message
            << " at line: " << exception.getLineNumber ()
            << std::endl;
  xercesc::XMLString::release (&message);
}

/*SaxHandler::SaxHandler(const SaxHandler& orig)
{

}*/

SaxHandler::~SaxHandler ()
{
}

}
