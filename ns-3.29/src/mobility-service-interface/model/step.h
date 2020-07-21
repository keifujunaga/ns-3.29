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

#ifndef STEP_H
#define STEP_H
#include <list>

#include <ns3/object.h>
#include <ns3/log.h>
#include "point.h"
namespace ns3 {
/**
* \ingroup mobility
 *
* \brief Represents a step, which is a section of a given journey
*
*  Steps are composed by points. It also contains information regarding the time to complete the step.
*  The XML from the Google Maps Directions Service returns a polyline. A polyline is a set of encoded latitude/longitude pairs (in practice, multiple points), which needs to be decoded in order to be used with this module.
*  This class is part of the module's model architecture, comprised of Legs, which contain Steps which, in turn, contain Points.
*/
class Step : public ns3::Object
{
public:
  /**
    * \brief Creates a step
   *
    * Creates a step with its attributes set to 0
    */
  Step ();
  /**
    * \param polyline The polyline which contains the encoded points of the step
    * \param travelTime The time it takes to complete this particular step
   *
    * Create a step
    */
  Step (std::string polyline, int travelTime);
  /**
  * \param orig Step to copy
   *
  * Standard copy constructor
  */
  Step (const Step& orig);
  virtual ~Step ();
  /**
   * \brief Get the list of points in that step
   *
   * \return The list of points
   */
  std::list<Ptr<Point> > GetPointList ();
  /**
   * \brief Get the polyline of the step
   *
   * \return a string which represents the polyline for that step
   */
  std::string GetPolyline ();
  /**
   * \brief Get the travel time of the step
   *
   * \return m_polyline a double which represents the total time it takes to travel the step
   */
  double GetTravelTime ();
  void SetPointList (std::list<Ptr<Point> > pointList);
private:
  std::string m_polyline;
  std::list<Ptr<Point> > m_pointList;
  double m_travelTime;   //in seconds
  int EscapeSpecialChars (std::string& polyline);
};

typedef std::list<Ptr<Step> > StepList;
std::ostream& operator<< (std::ostream& out, Step &p);

}
#endif  /* STEP_H */

