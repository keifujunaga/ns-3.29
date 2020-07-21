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
#include "step.h"
#include "ns3/step.h"
NS_LOG_COMPONENT_DEFINE ("Step");
namespace ns3 {
Step::Step ()
{
  NS_LOG_FUNCTION (this);
  this->m_polyline = "";
  this->m_travelTime = 0;
}
Step::Step (std::string polyline, int travelTime)
{
  NS_LOG_FUNCTION (this);
  this->m_polyline = polyline;
  this->m_travelTime = travelTime;
}
Step::Step (const Step& orig)
{
  this->m_polyline = orig.m_polyline;
  this->m_travelTime = orig.m_travelTime;
  this->m_pointList = orig.m_pointList;
}

Step::~Step ()
{
  NS_LOG_FUNCTION (this);
}
std::list<Ptr<Point> >
Step::GetPointList ()
{
  NS_LOG_FUNCTION (this);
  return m_pointList;
}
double
Step::GetTravelTime ()
{
  NS_LOG_FUNCTION (this);
  return m_travelTime;
}
std::string
Step::GetPolyline ()
{
  NS_LOG_FUNCTION (this);
  return m_polyline;
}

void
Step::SetPointList (std::list<Ptr<Point> > pointList)
{
  NS_LOG_FUNCTION (this);
  m_pointList = pointList;
}


std::ostream&
operator<< (std::ostream& out, Step &s)
{
  std::list<Ptr<Point> > pl = s.GetPointList ();
  if (pl.empty ())
    {
      return out;
    }
  int j = 0;
  out << "Step: " << std::endl;
  for (std::list<Ptr<Point> >::const_iterator ci = pl.begin(); ci != pl.end(); ++ci)
    {
      Ptr<Point> p = (*ci);
      out << "Point " << j << ":" << std::endl;
      out << *p;
      j++;
    }
  return out;
}

}
