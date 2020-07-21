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

#include "point.h"
NS_LOG_COMPONENT_DEFINE("Point");
namespace ns3 {

Point::Point()
  {
  NS_LOG_FUNCTION(this);
  this->m_lat=0;
  this->m_lng=0;
  this->m_waypointTime=0;
  this->m_x=0;
  this->m_y=0;
  this->m_z=0;
  }
Point::Point (double lat, double lng)
  {
  NS_LOG_FUNCTION(this);
  this->m_lat = lat;
  this->m_lng = lng;
  this->m_waypointTime = 0;
  }

Point::Point (const Point& orig)
  {
  this->m_lat = orig.m_lat;
  this->m_lng = orig.m_lng;
  this->m_waypointTime = orig.m_waypointTime;
  this->m_x = orig.m_x;
  this->m_y = orig.m_y;
  this->m_z = orig.m_z;
  }

Point::~Point () 
  {
  NS_LOG_FUNCTION(this);
  }

double
Point::GetXCoordinate ()
  {
  NS_LOG_FUNCTION(this);
  return m_x;
  }

double
Point::GetYCoordinate ()
  {
  NS_LOG_FUNCTION(this);
  return m_y;
  }

double
Point::GetZCoordinate ()
  {
  NS_LOG_FUNCTION(this);
  return m_z;
  }

double
Point::GetLatitude ()
  {
  NS_LOG_FUNCTION(this);
  return m_lat;
  }

double
Point::GetLongitude ()
  {
  NS_LOG_FUNCTION(this);
  return m_lng;
  }

void
Point::SetXCoordinate (double X)
  {
  NS_LOG_FUNCTION(this);
  m_x = X;
  }

void
Point::SetYCoordinate (double Y)
  {
  NS_LOG_FUNCTION(this);
  m_y = Y;
  }

void
Point::SetZCoordinate (double Z)
  {
  NS_LOG_FUNCTION(this);
  m_z = Z;
  }

void
Point::SetWaypointTime (double time)
  {
  NS_LOG_FUNCTION(this);
  this->m_waypointTime = time;
  }

double
Point::GetWaypointTime ()
  {
  NS_LOG_FUNCTION(this);
  return m_waypointTime;
  }
bool
Point::operator==(Point& p)
  {
  if(m_waypointTime==p.GetWaypointTime() && m_x == p.GetXCoordinate() && m_y == p.GetYCoordinate() && m_z == p.GetZCoordinate() && m_lat == p.GetLatitude () && m_lng == p.GetLongitude ())
    {
    return true;
    }
  return false;
  }
std::ostream& 
operator<< (std::ostream& out, Point &p)
  {
  out.precision (15);
  out << "Latitude:" << p.GetLatitude () << std::endl << "Longitude: "<< p.GetLongitude () << std::endl << "X: " << p.GetXCoordinate () << std::endl << "Y: " << p.GetYCoordinate () << std::endl << "Z:" << p.GetZCoordinate () << std::endl << "Time:"<< p.GetWaypointTime () << std::endl<< "---------------------" << std::endl;
  return out;
  }
  }
  