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

#ifndef POINT_H
#define POINT_H

#include <ns3/object.h>
#include <ns3/log.h>
#include <ostream>
namespace ns3 {

/**
* \ingroup mobility
* \brief Represents a point in time and space
*
* It comprises both the cartesian and the geodetic coordinates of the point
* This class is part of the model architecture for the module, which comprises Legs, which contain Steps and these, in turn, contain Points.
* All Points are calculated from a polyline, which is an attribute of Step and is returned by the Google Maps Directions Service.
*/

class Point : public Object
{
public:
  /**
    * \brief Creates a point.
    *
    * Creates a point with its attributes set to 0
    */
  Point ();
  /**
   * \param lat latitude of the point
   * \param lng longitude of the point
   *
   * Create a point.
   */
  Point (double lat, double lng);
  /**
   * \param orig Point to copy
   *
   * Standard copy constructor
   */
  Point (const Point& orig);
  virtual ~Point ();
  /**
   * \brief Get the X coordinate of the point
   *
   * \return a double which represents the X coordinate of the point
   */
  double GetXCoordinate ();
  /**
   * \brief Get the Y coordinate of the point
   *
   * \return a double which represents the Y coordinate of the point
   */
  double GetYCoordinate ();
  /**
   * \brief Get the Z coordinate of the point
   *
   * \return a double which represents the Z coordinate of the point
   */
  double GetZCoordinate ();
  /**
   * \brief Get the latitude coordinate of the point
   *
   * \return a double which represents the latitude of the point
   */
  double GetLatitude ();
  /**
   * \brief Get the longitude coordinate of the point
   *
   * \return a double which represents the longitude of the point
   */
  double GetLongitude ();
  /**
   * \brief Get the time related to the given point
   *
   * \return a double which represents the X coordinate of the point
   */
  double GetWaypointTime ();
  /**
   * \brief Set the X coordinate of the point
   *
   * \param X a double which will be set as the point's X coordinate
   */
  void SetXCoordinate (double X);
  /**
   * \brief Set the Y coordinate of the point
   *
   * \param Y a double which will be set as the point's Y coordinate
   */
  void SetYCoordinate (double Y);
  /**
   * \brief Set the Z coordinate of the point
   *
   * \param Z a double which will be set as the point's Z coordinate
   */
  void SetZCoordinate (double Z);
  /**
   * \brief Set the time of the point
   *
   * \param time a double which will be set as the point's time
   */
  void SetWaypointTime (double time);
  bool operator==(Point& p);
private:
  double m_lat;//!< The latitude coordinate for the point, in WSG84 format
  double m_lng;//!< The longitude coordinate for the point, in WSG84 format
  double m_x;//!< The X axis coordinate for the point
  double m_y;//!< The Y axis coordinate for the point
  double m_z;//!< The Z axis coordinate for the point
  double m_waypointTime;//!< The waypoint time for that point, in seconds


};
std::ostream& operator<< (std::ostream& out, Point &p);
}

#endif  /* POINT_H */

