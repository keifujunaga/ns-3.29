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

#include "place.h"
NS_LOG_COMPONENT_DEFINE("Place");
namespace ns3{
Place::Place () 
  {
  NS_LOG_FUNCTION(this);
  this->m_lat=0;
  this->m_lng=0;
  this->m_nextPageToken="";
  }
Place::Place(double lat, double lng)
  {
  NS_LOG_FUNCTION(this);
  this->m_lat=lat;
  this->m_lng=lng;
  this->m_nextPageToken="";
  }

Place::Place (const Place& orig) 
  {
  this->m_lat=orig.m_lat;
  this->m_lng=orig.m_lng;
  this->m_nextPageToken=orig.m_nextPageToken;
  }

Place::~Place () 
  {
  NS_LOG_FUNCTION(this);
  }
double
Place::GetLatitude ()
  {
  NS_LOG_FUNCTION(this);
  return m_lat;
  }
double
Place::GetLongitude ()
  {
  NS_LOG_FUNCTION(this);
  return m_lng;
  }
void
Place::SetNextPageToken (std::string nextPageToken)
  {
  NS_LOG_FUNCTION(this);
  this->m_nextPageToken=nextPageToken;
  }
std::string
Place::GetNextPageToken ()
  {
  NS_LOG_FUNCTION(this);
  return m_nextPageToken;
  }
std::ostream& 
operator<< (std::ostream& out, Place &p)
  {
  out.precision (5);
  out << "Latitude:" << p.GetLatitude () << std::endl << "Longitude: "<< p.GetLongitude () << std::endl << "PageToken:"<< p.GetNextPageToken () << std::endl<< "---------------------" << std::endl;
  return out;
  }

}//namespace ns3