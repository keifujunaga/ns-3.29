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

#include <GeographicLib/Geocentric.hpp>
#include <GeographicLib/LocalCartesian.hpp>
#include <list>
#include "ns3/simulator.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/routes-mobility-helper.h"
#include "ns3/test.h"

/**
 * \brief Tests the RoutesMobilityModel conversion capabilities
 *
 * This test performs comparisons between latitude and longitude values obtained from Google's Polyline Encoder Utility and values obtained from the conversion algorithms implemented in the module
 * (Google's Polyline enconder utility - https://developers.google.com/maps/documentation/utilities/polylineutility )
 *
 * It also tests the Cartesian coordinates obtained from the GeographicLib library conversion functions.
 *
 */

namespace ns3 {

class RoutesMobilityModelTest : public TestCase
{
public:
  RoutesMobilityModelTest () : TestCase ("Check RoutesMobilityModel conversion capabilities")
  {
    m_gmapsStrategy = new GoogleMapsApiConnect (41.1621429, -8.6218531,0);
  }
  virtual ~RoutesMobilityModelTest ()
  {
  }

private:
  GoogleMapsApiConnect* m_gmapsStrategy;
  virtual void DoRun (void);
};

void
RoutesMobilityModelTest::DoRun (void)
{
  Ptr<Leg> leg;
  Ptr<Step> step;
  std::list<Ptr<Leg> > legList;
  std::list<Ptr<Step> > stepList;
  std::list<Ptr<Point> > pointList, comparePointList, auxCmpPointList;
  std::list<Ptr<Step> >::iterator stepIt;
  legList = m_gmapsStrategy->PerformOfflineRequest ("src/mobility-service-interface/test/routes-mobility-model-xml-test.xml");

  //Create the expected values
  Ptr<Point> p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18,p19;
  p1 = new Point (41.18063,-8.609350000000001);
  p2 = new Point (41.17956, -8.60930000000001);
  p3 = new Point (41.17938, -8.60930000000001);
  p4 = new Point (41.178790000000006, -8.609250000000001);
  p5 = new Point (41.17817, -8.60923);
  p6 = new Point (41.17734, -8.60918);
  p7 = new Point (41.17734, -8.60918);
  p8 = new Point (41.17730, -8.60897);
  p9 = new Point (41.17718, -8.60864);
  p10 = new Point (41.17697, -8.60815);
  p11 = new Point (41.17694, -8.60809);
  p12 = new Point (41.17837, -8.60624);
  p13 = new Point (41.17830, -8.60614);
  p14 = new Point (41.17746, -8.60496);
  p15 = new Point (41.17688, -8.60412);
  p16 = new Point (41.17676, -8.60400);
  p17 = new Point (41.17666, -8.60394);
  p18 = new Point (41.17653, -8.60390);
  p19 = new Point (41.17640, -8.60389);

  p1->SetXCoordinate (1049.07);
  p1->SetYCoordinate (2053.2);
  p1->SetZCoordinate (-0.417406);

  p2->SetXCoordinate (1053.29);
  p2->SetYCoordinate (1934.37);
  p2->SetZCoordinate (-0.380866);

  p3->SetXCoordinate (1053.29);
  p3->SetYCoordinate (1914.38);
  p3->SetZCoordinate (-0.374821);

  p4->SetXCoordinate (1057.49);
  p4->SetYCoordinate (1848.86);
  p4->SetZCoordinate (-0.35614);

  p5->SetXCoordinate (1059.18);
  p5->SetYCoordinate (1780.0);
  p5->SetZCoordinate (-0.336785);

  p6->SetXCoordinate (1063.39);
  p6->SetYCoordinate (1687.82);
  p6->SetZCoordinate (-0.312367);

  p7->SetXCoordinate (1063.4);
  p7->SetYCoordinate (1687.8);
  p7->SetZCoordinate (-0.31237);

  p8->SetXCoordinate (1081);
  p8->SetYCoordinate (1683.4);
  p8->SetZCoordinate (-0.31415);

  p9->SetXCoordinate (1108.7);
  p9->SetYCoordinate (1670.1);
  p9->SetZCoordinate (-0.31538);

  p10->SetXCoordinate (1149.8);
  p10->SetYCoordinate (1646.7);
  p10->SetZCoordinate (-0.31658);

  p11->SetXCoordinate (1154.9);
  p11->SetYCoordinate (1643.4);
  p11->SetZCoordinate (-0.31663);

  p12->SetXCoordinate (1310.1);
  p12->SetYCoordinate (1802.3);
  p12->SetZCoordinate (-0.38958);

  p13->SetXCoordinate (1318.5);
  p13->SetYCoordinate (1794.5);
  p13->SetZCoordinate (-0.38911);

  p14->SetXCoordinate (1417.5);
  p14->SetYCoordinate (1701.2);
  p14->SetZCoordinate (-0.3847);

  p15->SetXCoordinate (1488);
  p15->SetYCoordinate (1636.8);
  p15->SetZCoordinate (-0.38384);

  p16->SetXCoordinate (1498.1);
  p16->SetYCoordinate (1623.5);
  p16->SetZCoordinate (-0.38278);

  p17->SetXCoordinate (1503.1);
  p17->SetYCoordinate (1612.4);
  p17->SetZCoordinate (-0.38114);

  p18->SetXCoordinate (1506.5);
  p18->SetYCoordinate (1597.9);
  p18->SetZCoordinate (-0.37829);

  p19->SetXCoordinate (1507.3);
  p19->SetYCoordinate (1583.5);
  p19->SetZCoordinate (-0.37488);

  p1->SetWaypointTime (0.0);
  p2->SetWaypointTime (13.3308);
  p3->SetWaypointTime (15.572);
  p4->SetWaypointTime (22.9331);
  p5->SetWaypointTime (30.655);
  p6->SetWaypointTime (41.0);
  p7->SetWaypointTime (41.001);
  p8->SetWaypointTime (44.556);
  p9->SetWaypointTime (50.569);
  p10->SetWaypointTime (59.819);
  p11->SetWaypointTime (61.0);
  p12->SetWaypointTime (113);
  p13->SetWaypointTime (114.06);
  p14->SetWaypointTime (126.71);
  p15->SetWaypointTime (135.59);
  p16->SetWaypointTime (137.14);
  p17->SetWaypointTime (138.28);
  p18->SetWaypointTime (139.66);
  p19->SetWaypointTime (141.0);
  //Create the point list
  pointList.push_back (p1);
  pointList.push_back (p2);
  pointList.push_back (p3);
  pointList.push_back (p4);
  pointList.push_back (p5);
  pointList.push_back (p6);
  pointList.push_back (p7);
  pointList.push_back (p8);
  pointList.push_back (p9);
  pointList.push_back (p10);
  pointList.push_back (p11);
  pointList.push_back (p12);
  pointList.push_back (p13);
  pointList.push_back (p14);
  pointList.push_back (p15);
  pointList.push_back (p16);
  pointList.push_back (p17);
  pointList.push_back (p18);
  pointList.push_back (p19);

  leg = (legList.front ());
  leg->GetStepList (stepList);
  step = stepList.front ();
  comparePointList = step->GetPointList ();
  stepIt = stepList.begin ();
  //Move to the next step
  stepIt++;
  step = (*stepIt);
  auxCmpPointList = step->GetPointList ();
  //Add the step's points to the compare list
  for (std::list<Ptr<Point> >::iterator i = auxCmpPointList.begin (); i != auxCmpPointList.end (); i++)
    {
      comparePointList.push_back ((*i));
    }
  //Move two steps
  stepIt++;
  stepIt++;
  step = (*stepIt);
  auxCmpPointList = step->GetPointList ();
  //Add the step's points to the compare list
  for (std::list<Ptr<Point> >::iterator i = auxCmpPointList.begin (); i != auxCmpPointList.end (); i++)
    {
      comparePointList.push_back ((*i));
    }

  //The compare list now has the points from 3 full steps, the first two steps are consecutive.

  for (std::list<Ptr<Point> >::iterator i = comparePointList.begin (),j = pointList.begin (); i != comparePointList.end (); i++,j++)
    {
      NS_TEST_ASSERT_MSG_EQ_TOL ((*i)->GetLatitude (), (*j)->GetLatitude (), 0.1, "The points do not match!");
      NS_TEST_ASSERT_MSG_EQ_TOL ((*i)->GetLongitude (), (*j)->GetLongitude (), 0.1, "The points do not match!");
      NS_TEST_ASSERT_MSG_EQ_TOL ((*i)->GetXCoordinate (), (*j)->GetXCoordinate (), 0.1, "The points do not match!");
      NS_TEST_ASSERT_MSG_EQ_TOL ((*i)->GetYCoordinate (), (*j)->GetYCoordinate (), 0.1, "The points do not match!");
      NS_TEST_ASSERT_MSG_EQ_TOL ((*i)->GetZCoordinate (), (*j)->GetZCoordinate (), 0.1, "The points do not match!");
      NS_TEST_ASSERT_MSG_EQ_TOL ((*i)->GetWaypointTime (), (*j)->GetWaypointTime (), 0.1, "The points do not match!");
    }

  Simulator::Stop (Seconds (65));
  Simulator::Run ();
  Simulator::Destroy ();
}

static struct RoutesMobilityModelTestSuite : public TestSuite
{
  RoutesMobilityModelTestSuite () : TestSuite ("routes-mobility-model", UNIT)
  {
    AddTestCase (new RoutesMobilityModelTest (), TestCase::QUICK);
  }
} g_routesMobilityModelTestSuite;

} // namespace ns3

