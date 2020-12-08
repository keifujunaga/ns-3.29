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

#ifndef LEG_H
#define LEG_H
#include <math.h>
#include <list>
#include <ns3/object.h>
#include "step.h"

namespace ns3 {

/**
* \ingroup mobility
* \brief Represents a Leg (a Leg is a Google Maps Waypoint), which is comprised of Steps
*
* This class is part of the module's model architecture, where a journey comprises Legs, which contain Steps which, in turn, contain Points.
* As explained before, Legs are a Google Maps Waypoint. For example, a journey containing only a start and an endpoint will have ony one Leg (because Google Maps considers that only one Waypoint exists). If the user specifies a route like A to B, passing through C and D, for example, the Google Maps Directions Service structures the information into 3 Legs, which contain (each one), multiple Steps and these, in turn, contain the information of all the points in that Step.
* At the moment, the functionality to request a route with Waypoints (A to B, passing through C,D,E, for example) from Google Maps is not yet implemented.
* The ns3::WaypointMobilityModel uses a dequeue to store the ns3::Waypoint of a node, which means that the whole journey information only needs to be passed to the corresponding dequeue.
*/
class Leg : public Object
{
public:
  Leg ();
  /**
   * \param stepList A list of steps contained in the Leg
   *
   * Create a leg
   */
  Leg (StepList &stepList);
  Leg (const Leg& orig);
  virtual ~Leg ();
  /**
   * \brief Calculate the total distance travelled in this Step
   *
   *  This function is responsible for calculating the total distance spanned by the step
   * \param step A step
   *
   * \return a double which represents the total distance spanned by the step, in the cartesian plane
   */
  double CalculateTotalDistanceInStep (Step& step);
  /**
  * \brief Get the StepList of this leg
   *
  * This function returns the StepList of this leg by reference
  * \param stepList A StepList
  */
  void GetStepList (StepList &stepList);
  /**
   * \brief Set the StepList of this leg
   *
   * \param stepList
   */
  void SetStepList (std::list<Ptr<Step> > stepList);
private:
  /**
   * \brief Calculate the distance between two points
   *
   *  This function is responsible for calculating the total distance between two points
   * \param p1
   * \param p2
   *
   * \return a double which represents the total distance between the two points, in the cartesian plane
   */
  double CalculatePartialDistance (Ptr<Point> p1, Ptr<Point> p2);
  /**
   *\brief A list of Step
   */
  StepList m_stepList;
};
typedef std::list<Ptr<Leg> > LegList;
}
#endif  /* LEG_H */

