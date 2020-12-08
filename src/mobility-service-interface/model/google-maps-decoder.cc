
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

#include "google-maps-decoder.h"
NS_LOG_COMPONENT_DEFINE ("GoogleMapsDecoder");
namespace ns3 {

GoogleMapsDecoder::GoogleMapsDecoder ()
{
  NS_LOG_FUNCTION (this);
  m_earth = GeographicLib::Geocentric (GeographicLib::Constants::WGS84_a (), GeographicLib::Constants::WGS84_f ());
  m_proj = GeographicLib::LocalCartesian (41.1621429, -8.6218531, 0, m_earth);
}

GoogleMapsDecoder::GoogleMapsDecoder (double lat, double lng, double altitude)
{
  NS_LOG_FUNCTION (this);
  m_earth = GeographicLib::Geocentric (GeographicLib::Constants::WGS84_a (), GeographicLib::Constants::WGS84_f ());
  m_proj = GeographicLib::LocalCartesian (lat, lng, altitude, m_earth);
}
GoogleMapsDecoder::GoogleMapsDecoder (const GoogleMapsDecoder& orig)
{
}

GoogleMapsDecoder::~GoogleMapsDecoder ()
{
  NS_LOG_FUNCTION (this);
}

int
GoogleMapsDecoder::ConvertToGeoCoordinates (std::string polyline, std::list<Ptr<Point> > &pointList)
{
  NS_LOG_FUNCTION (this);
  int index = 0;
  int currentLat = 0;
  int currentLng = 0;
  int next5bits;
  int sum;
  int shifter;
  int polylineSize = polyline.length ();
  try
    {
      while (index < polylineSize)
        {
          //calculate next latitude
          sum = 0;
          shifter = 0;
          do
            {
              next5bits = (int) polyline.at (index++) - 63;
              sum |= (next5bits & 31) << shifter;
              shifter += 5;
            }
          while (next5bits >= 32 && index < polylineSize);
          if (index >= polylineSize)
            {
              break;
            }
          currentLat += (sum & 1) == 1 ? ~(sum >> 1) : (sum >> 1);

          //calculate next longitude
          sum = 0;
          shifter = 0;
          do
            {
              next5bits = int(polyline.at (index++) - 63);
              sum |= (next5bits & 31) << shifter;
              shifter += 5;
            }
          while (next5bits >= 32 && index < polylineSize);
          if (index >= polylineSize && next5bits >= 32)
            {
              break;
            }

          currentLng += (sum & 1) == 1 ? ~(sum >> 1) : (sum >> 1);
          //Point *p = new Point(currentLat/100000.0, currentLng/100000.0);
          Ptr<Point> p = CreateObject <Point> (currentLat / 100000.0, currentLng / 100000.0);
          //p->WriteToStdOut ();
          pointList.push_back (p);
        }
    }
  catch (int ex)
    {
      return ex;
    }
  return 1;
}

int
GoogleMapsDecoder::ConvertToCartesian (LegList& legList)
{
  NS_LOG_FUNCTION (this);
  double totalDistanceInStep = 0;
  double x = 0, y = 0, z = 0, h = 0;
  StepList stepList;
  Ptr<Point> pTemp;
  Ptr<Leg> leg;
  std::list<Ptr<Point> > pointList, tempPointList;
  std::list<Ptr<Point> >::iterator tempPointIterator;
  for (LegList::iterator i = legList.begin (); i != legList.end (); i++)
    {
      leg = (*i);
      leg->GetStepList (stepList);
      for (StepList::iterator j = stepList.begin (); j != stepList.end (); j++)
        {
          pointList = (*j)->GetPointList ();
          ConvertToGeoCoordinates ((*j)->GetPolyline (), pointList);
          (*j)->SetPointList (pointList);
          if((*j)->GetTravelTime ()==0)
            {
            pTemp = new Point(*pointList.back ());
            pointList.clear ();
            pointList.push_front (pTemp);
            (*j)->SetPointList(pointList);
            }
          for (std::list <Ptr<Point> >::iterator k = pointList.begin (); k != pointList.end (); k++)
            {
              pTemp = (*k);
              m_proj.Forward ((pTemp->GetLatitude ()), (pTemp->GetLongitude ()), h, x, y, z);
              pTemp->SetXCoordinate (x);
              pTemp->SetYCoordinate (y);
              pTemp->SetZCoordinate (z);
            }
          totalDistanceInStep = leg->CalculateTotalDistanceInStep (*(*j));
          FillInWaypointTime (pointList, totalDistanceInStep, stepList.begin (),j, legList.begin (),i,(*j)->GetTravelTime () );
        }
    }
  return 0;
}

int
GoogleMapsDecoder::SetWaypointTime (double startAt, double totalDistanceInStep, Ptr<Point> p1, Ptr<Point> p2, double travelTime)
{
  NS_LOG_FUNCTION (this);
  double timeElapsed = 0;
  double partialDistance = 0;
	double waypointTime = 0;
  if (startAt >= 0)
    {
      partialDistance = CalculatePartialDistance (p1->GetXCoordinate (), p2->GetXCoordinate (), p1->GetYCoordinate (), p2->GetYCoordinate ());
      timeElapsed = (partialDistance * travelTime) / totalDistanceInStep;
      waypointTime = (startAt + timeElapsed);
      p2->SetWaypointTime (waypointTime);
      NS_LOG_DEBUG("The time of the current waypoint is: " << waypointTime << " seconds.");
    }
  else
   {
      waypointTime = 0;
      p1->SetWaypointTime (waypointTime);
      NS_LOG_DEBUG("This is the first point in the list, and its time was set to zero seconds");

    }
  return waypointTime;
}

double
GoogleMapsDecoder::CalculatePartialDistance (double x1, double x2, double y1, double y2)
{
  NS_LOG_FUNCTION (this);
  double distance = sqrt (((pow ((x2 - x1), 2.0)) + (pow ((y2 - y1), 2.0))));
  return distance;
}

void
GoogleMapsDecoder::FillInWaypointTime (std::list<Ptr<Point> > &pointList, double totalDistanceInStep, std::list<Ptr<Step> >::iterator stepListBegin, std::list<Ptr<Step> >::iterator currentStepPosition, std::list<Ptr<Leg> >::iterator legListBegin, std::list<Ptr<Leg> >::iterator currentLegPosition, double travelTime)
{
  NS_LOG_FUNCTION (this);
  std::list<Ptr<Step> >::iterator tempStepListIterator;
  std::list<Ptr<Leg> >::iterator tempLegIterator;
  std::list<Ptr<Step> > tempStepList;
  bool firstExecution = true;
  Ptr<Point> lastPoint,pTemp;
  std::list<Ptr<Leg> > legList;
  std::list<Ptr<Point> > tmpPointList;
  std::list<Ptr<Point> >::iterator lastPointIterator;
  double waypointTime = 0;
  for (std::list <Ptr<Point> >::iterator k = pointList.begin (); k != pointList.end (); k++)
    {
      pTemp = (*k);
      lastPointIterator = k;

      //First point gets set to 0 seconds
      if (firstExecution && currentLegPosition == legListBegin && currentStepPosition == stepListBegin)
        {
          SetWaypointTime (-1, totalDistanceInStep, pTemp, NULL, (*(*currentStepPosition)).GetTravelTime ());
          firstExecution = false;
        }
      else
        {
          //If it's not the first point in the list, we can check the last time by iterating the list back
          if (k != pointList.begin ())
            {
              lastPointIterator--;
              lastPoint = *lastPointIterator;
              waypointTime = SetWaypointTime ((lastPoint)->GetWaypointTime (), totalDistanceInStep, (lastPoint), pTemp, travelTime);
              if(waypointTime - (lastPoint->GetWaypointTime()) < 0)
                {
                NS_LOG_DEBUG(this << ": The following consecutive waypoints's time difference is less than zero!");
                NS_LOG_DEBUG("Previous waypoint:" << std::endl << *lastPoint << std::endl << "Current waypoint: " << *(*k));
                }
              waypointTime = 0;
            }
          else
            {
              //If it's the first point in the list, we need to check if it's at the start of its steplist
              if (currentStepPosition != stepListBegin)
                {
                  //If it's not, we can iterate back a step and check the time of the last point of the previous step.
                  tempStepListIterator = currentStepPosition;
                  //Iterate back a step
                  tempStepListIterator--;
                  //Get last point in that step
                  tmpPointList = (*tempStepListIterator)->GetPointList ();
                  unsigned int size = tmpPointList.size();
                  if(size==1)
                    {
                    lastPoint = tmpPointList.front ();
                    }
                  else
                    {
                    lastPoint = (tmpPointList.back ());
                    }
                  waypointTime = SetWaypointTime ((lastPoint)->GetWaypointTime (), totalDistanceInStep, (lastPoint), pTemp, travelTime);
                  if(waypointTime - (lastPoint->GetWaypointTime()) < 0)
                    {
                    NS_LOG_DEBUG(this << ": The following consecutive waypoints's time difference is less than zero!");
                    NS_LOG_DEBUG("Previous waypoint:" << std::endl << *lastPoint << std::endl << "Current waypoint: " << *(*k));
                    }
                  waypointTime = 0;
                }
              else
                {
                  //If we are at the start of the steplist we need to iterate back a leg and check its last step and the last point in that step
                  if (currentLegPosition != legListBegin)
                    {
                      tempLegIterator = currentLegPosition;
                      //Iterate back a leg
                      tempLegIterator--;
                      //Get the steplist in that leg
                      (*tempLegIterator)->GetStepList (tempStepList);
                      //Get the last point of the last step of the previous leg.
                      lastPoint = ((*tempStepList.back ()).GetPointList ()).back ();
                      waypointTime = SetWaypointTime ((lastPoint)->GetWaypointTime (), totalDistanceInStep, (lastPoint), pTemp, travelTime);
                      if(waypointTime - (lastPoint->GetWaypointTime()) < 0)
                        {
                        NS_LOG_DEBUG(this << ": The following consecutive waypoints's time difference is less than zero!");
                        NS_LOG_DEBUG("Previous waypoint:" << std::endl << *lastPoint << std::endl << "Current waypoint: " << *(*k));
                        }
                      waypointTime = 0;
                    }
                }
            }
        }
    }
}
} //namespace ns3

