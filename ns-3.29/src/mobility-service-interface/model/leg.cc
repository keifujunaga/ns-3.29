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

#include "leg.h"
NS_LOG_COMPONENT_DEFINE ("Leg");
namespace ns3 {
Leg::Leg ()
{
  NS_LOG_FUNCTION (this);
}

Leg::Leg (StepList &stepList)
{
  NS_LOG_FUNCTION (this);
  this->m_stepList = stepList;
}

Leg::Leg (const Leg& orig)
{
}

Leg::~Leg ()
{
  NS_LOG_FUNCTION (this);
}

double
Leg::CalculatePartialDistance (Ptr<Point> p1, Ptr<Point> p2)
{
  NS_LOG_FUNCTION (this);
  double distance = 0;
  distance = sqrt (((pow ((p2->GetXCoordinate () - p1->GetXCoordinate ()), 2.0)) + (pow ((p2->GetYCoordinate () - p1->GetYCoordinate ()), 2.0))));
  return distance;
}

double
Leg::CalculateTotalDistanceInStep (Step& step)
{
  NS_LOG_FUNCTION (this);
  double distance = 0;
  std::list<Ptr<Point> > pointList = step.GetPointList ();
  std::list<Ptr<Point> >::iterator i = pointList.begin ();
  i++;
  for (std::list<Ptr<Point> >::iterator k = pointList.begin (); i != pointList.end (); i++, k++)
    {
      distance += CalculatePartialDistance ((*k), (*i));
    }
  return distance;
}
void
Leg::GetStepList (StepList &stepList)
{
  NS_LOG_FUNCTION (this);
  stepList = StepList (m_stepList);
}
void
Leg::SetStepList (std::list<Ptr<Step> > stepList)
{
  NS_LOG_FUNCTION (this);
  this->m_stepList = StepList (stepList);
}
}
