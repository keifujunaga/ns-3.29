
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
#ifndef GOOGLEMAPSDECODER_H
#define GOOGLEMAPSDECODER_H
#include <string>
#include <iostream>
#include <list>
#include <GeographicLib/Geocentric.hpp>
#include <GeographicLib/LocalCartesian.hpp>

#include <ns3/pointer.h>

#include "leg.h"
namespace ns3 {

/**
 * \ingroup mobility
 * \brief This class is responsible for decoding the information received from the Google Maps Directions API
 *
 * The information received from the Google Maps Directions API cannot be used by ns3 as it is. The coordinates of the points are encoded in a format called a polyline.
 * A polyline is a string representation of a set of points. This class is responsible for decoding the polylines, convert from Geodetic coordinates (WSG84) to Cartesian coordinates and fill up the time information for the points of the polylines
 */
class GoogleMapsDecoder
{
public:
  /**
   * \brief Creates a GoogleMapsDecoder object
   *
   * Creates a GoogleMapsDecoder object, setting its attributes to the default values
   */
  GoogleMapsDecoder ();
  /**
   * \brief Creates a GoogleMapsDecoder object
   *
   * Creates a GoogleMapsDecoder object setting the center of the Cartesian plane to the parameters passed.
   *
   * \param lat The latitude of the point which will serve as the center of the Cartesian plane
   * \param lng The longitude of the point which will serve as the center of the Cartesian plane
   * \param altitude The altitude of the point which will serve as the center of the Cartesian plane
   */
  GoogleMapsDecoder (double lat, double lng, double altitude);
  /**
   * \brief Creates a GoogleMapsDecoder object
   *
   * Standard copy constructor
   * \param orig The object to copy from
   */
  GoogleMapsDecoder (const GoogleMapsDecoder& orig);
  virtual ~GoogleMapsDecoder ();
  /**
   * \brief Converts the latitude and longitude of all points into Cartesian coordinates
   *
   * This function takes a LegList and converts the latitude and longitude of all the points to Cartesian coordinates
   * \param legList
   */
  int ConvertToCartesian (LegList& legList);
private:
  GeographicLib::Geocentric m_earth;
  GeographicLib::LocalCartesian m_proj;
  /**
   * \brief Converts the polyline passed as the argument into Points
   *
   * This function fills the std::list<Ptr<Point> > passed to the function with all the points in the given polyline
   * \param polyline The polyline to decode
   * \param pointList The std::list to fill with Points
   */
  int ConvertToGeoCoordinates (std::string polyline, std::list< Ptr<Point> > &pointList);
  /**
   * \brief Calculates the distance between two points
   *
   * \param x1 The X coordinate of Point 1
   * \param x2 The X coordinate of Point 2
   * \param y1 The Y coordinate of Point 1
   * \param y2 The Y coordinate of Point 2
   * \return a double which represents the distance between the two points
   */
  double CalculatePartialDistance (double x1, double x2, double y1, double y2);
  //returns the last waypoint time. Maybe create a offset variable to separate steps in time, if needed
  /**
   * \brief Calculates the time at which this Point should be reached
   *
   * In order to add realism to the mobility trace, the time attribute of the points needs to be set realistically.
   * This function computes the time it would take for a node to reach the given point, in practice, creating motion that resembles its real-world counterpart.
   * \param startAt This attribute contains the time information from the previous point in the Step
   * \param totalDistanceInStep The total distance spanned by the Step. This value is calculated
   * \param p1 The previous Point of the std::list of Points
   * \param p2 This Point will receive the calculated time
   * \param travelTime The total time it takes to go through the Step. This attribute is originally received from the Google Maps API.
   * \return Returns 0 if the function exists without errors
   */
  int SetWaypointTime (double startAt, double totalDistanceInStep, Ptr<Point> p1, Ptr<Point> p2, double travelTime);
  /**
   * \brief Fills the time of all the Points in the std::list<Ptr<Point> >
   *
   * This function iterates through the std::list<Ptr<Point> > to set the time of all Points
   * \param pointList The list of Points
   * \param totalDistanceInStep The total distance of that Step
   * \param stepListBegin An iterator to the beginning of the StepList
   * \param currentStepPosition An iterator to the current Step position in the StepList
   * \param legListBegin An iterator to the beginning of the LegList
   * \param currentLegPosition An iterator to the current Leg position in the LegList
   * \param travelTime The total time it takes to go through this Step. This value is obtained directly from the Google Maps API.
   */
  void FillInWaypointTime (std::list<Ptr<Point> > &pointList, double totalDistanceInStep,
    std::list<Ptr<Step> >::iterator stepListBegin, std::list<Ptr<Step> >::iterator currentStepPosition,
    std::list<Ptr<Leg> >::iterator legListBegin, std::list<Ptr<Leg> >::iterator currentLegPosition, double travelTime);
};
} //namespace ns3
#endif  /* GOOGLEMAPSDECODER_H */

