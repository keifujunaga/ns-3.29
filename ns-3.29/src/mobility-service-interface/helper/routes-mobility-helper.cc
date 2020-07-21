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

#include "ns3/routes-mobility-helper.h"

NS_LOG_COMPONENT_DEFINE ("RoutesMobilityHelper");

namespace ns3 {

RoutesMobilityHelper::RoutesMobilityHelper ()
{
  NS_LOG_FUNCTION (this);
  srand (time (NULL));
  m_strategy = new GoogleMapsApiConnect ();
  // m_travelMode = "driving";
  m_travelMode = "walking";
  m_departureTime = GetCurrentTime ();
}

RoutesMobilityHelper::RoutesMobilityHelper (double lat, double lng, double altitude)
{
  NS_LOG_FUNCTION (this);
  srand (time (NULL));
  m_strategy = new GoogleMapsApiConnect (lat, lng,altitude);
  // m_travelMode = "driving";
  m_travelMode = "walking";
  m_departureTime = GetCurrentTime ();
}

RoutesMobilityHelper::RoutesMobilityHelper (const RoutesMobilityHelper& orig)
{
  this->m_strategy = orig.m_strategy;
  this->m_travelMode = orig.m_travelMode;
}

RoutesMobilityHelper::~RoutesMobilityHelper ()
{
  NS_LOG_FUNCTION (this);
  free (m_strategy);
}
std::string
RoutesMobilityHelper::GetCurrentTime ()
{
  NS_LOG_FUNCTION (this);
  std::ostringstream strs;
  std::time_t time = std::time (NULL);
  std::asctime (std::localtime (&time));
  strs << time;
  return strs.str ();
}
int
RoutesMobilityHelper::SchedulePoints (Ptr<Node> node, std::list<Ptr<Leg> > legList)
{
  NS_LOG_FUNCTION (this);
  int addedWaypoints = 0;
  std::list<Ptr<Step> > stepList;
  std::list<Ptr<Point> > pointList;
  Ptr<Point> p, previousPoint;
  bool first = false;
  Ptr<WaypointMobilityModel> waypointMobility = DynamicCast<WaypointMobilityModel> ( node->GetObject<MobilityModel> ());
  for (std::list<Ptr<Leg> >::iterator i = legList.begin (); i != legList.end (); i++)
    {
      (*i)->GetStepList (stepList);
      for (std::list<Ptr<Step> >::iterator j = stepList.begin (); j != stepList.end (); j++)
        {
          NS_LOG_DEBUG ("Iterating step " << *(*j));
          pointList = (*j)->GetPointList ();
          for (std::list<Ptr<Point> >::iterator k = pointList.begin (); k != pointList.end (); k++)
            {
              p = ((*k));
              if (first)
                {
                  p->SetWaypointTime (((p->GetWaypointTime ()) + 0.001));
                  first = false;
                }
              waypointMobility->AddWaypoint (Waypoint (Seconds (p->GetWaypointTime ()),Vector (p->GetXCoordinate (),p->GetYCoordinate (),p->GetZCoordinate ())));
              NS_LOG_DEBUG (*p << "Waypoint number " << (addedWaypoints + 1) << " added to Node " << node->GetId () << std::endl << "---------------------");
              addedWaypoints++;
            }
          first = true;
        }
    }
  NS_ASSERT_MSG (addedWaypoints != 0, "No waypoints were added");
  NS_LOG_INFO ("Added " << addedWaypoints << " waypoints to Node " << node->GetId ());
  return addedWaypoints;
}

int
RoutesMobilityHelper::ChooseRoute (std::string startPoint, std::string endPoint, Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);
  int addedWaypoints = 0;
  std::list<Ptr<Leg> > legList;
  legList = m_strategy->PerformRequest (startPoint,endPoint, m_travelMode, m_departureTime);
  if (g_log.IsEnabled (LOG_DEBUG))
    {
      uint32_t i = 0;
      for (std::list<Ptr<Leg> >::iterator list_iter = legList.begin ();
           list_iter != legList.end (); list_iter++, i++)
        {
          NS_LOG_DEBUG ("Leg " << i << ": " << *list_iter);
        }
    }
  addedWaypoints = SchedulePoints (node,legList);
  return addedWaypoints;
}

int
RoutesMobilityHelper::ChooseRoute (std::string const *listTokenStartEndPoint, NodeContainer &nodeContainer)
{
  NS_LOG_FUNCTION (this);
  int sizeOfContainer = nodeContainer.GetN ();
  std::string start = "", end = "";
  Ptr<Node> node;
  for (int i = 0, j = 0; i < sizeOfContainer; i++, j++)
    {
      node = nodeContainer.Get (i);
      start = listTokenStartEndPoint[j];
      j++;
      end = listTokenStartEndPoint[j];
      ChooseRoute (start, end, node);
    }
  return 0;
}
int
RoutesMobilityHelper::ChooseRoute (Ptr<Node> node, std::string path)
{
  NS_LOG_FUNCTION (this);
  std::list<Ptr<Leg> > legList;
  legList = m_strategy->PerformOfflineRequest (path);
  SchedulePoints (node,legList);
  return 0;
}

int
RoutesMobilityHelper::ChooseRoute (NodeContainer &nodeContainer, double lat, double lng, double radius)
{
  NS_LOG_FUNCTION (this);
  unsigned int numOfNodes = nodeContainer.GetN ();
  std::string start = "",endpoint = "";
  GoogleMapsPlacesApiConnect places;
  Ptr<Place> place;
  int startIndex = 0, endIndex = 0;
  std::vector<Ptr<Place> > placeArray;
  placeArray = places.PerformRequest (lat,lng,radius);
  for (unsigned int i = 0; i < numOfNodes; i++)
    {
      do
        {
          startIndex = (int) GenerateRandomValue (0, (placeArray.size () - 1));
          endIndex = (int) GenerateRandomValue (0, (placeArray.size () - 1));
        }
      while (startIndex == endIndex);
      start = LocationToString (placeArray[startIndex]->GetLatitude (), placeArray[startIndex]->GetLongitude ());
      endpoint = LocationToString (placeArray[endIndex]->GetLatitude (), placeArray[endIndex]->GetLongitude ());
      ChooseRoute (start, endpoint, nodeContainer.Get (i));
    }
  return 0;
}

int
RoutesMobilityHelper::ChooseRoute (NodeContainer &nodeContainer, double lat, double lng, double radius, double destLat, double destLng, double destRadius)
{
  NS_LOG_FUNCTION (this);
  unsigned int numOfNodes = nodeContainer.GetN ();
  std::string start = "",endpoint = "";
  GoogleMapsPlacesApiConnect places;
  std::vector<Ptr<Place> > placeArray, destPlaceArray;
  Ptr<Place> place;
  int startIndex = 0, endIndex = 0;
  placeArray = places.PerformRequest (lat,lng,radius);
  destPlaceArray = places.PerformRequest (destLat, destLng, destRadius);
  for (unsigned int i = 0; i < numOfNodes; i++)
    {
      startIndex = (int) GenerateRandomValue (0, (placeArray.size () - 1));
      endIndex = (int) GenerateRandomValue (0, (destPlaceArray.size () - 1));
      start = LocationToString (placeArray[startIndex]->GetLatitude (), placeArray[startIndex]->GetLongitude ());
      endpoint = LocationToString (destPlaceArray[endIndex]->GetLatitude (), destPlaceArray[endIndex]->GetLongitude ());
      ChooseRoute (start, endpoint, nodeContainer.Get (i));
    }
  return 0;
}

int
RoutesMobilityHelper::ChooseRoute (NodeContainer &nodeContainer, std::string dirPath)
{
  NS_LOG_FUNCTION (this);
  std::list<std::string> files;
  files = Open (dirPath);
  int nodeContainerSize = nodeContainer.GetN ();
  NS_ASSERT_MSG (files.size () == nodeContainer.GetN (), "The number of nodes is not equal to the number of XML files. Number of files is: " << files.size ());
  int j = 0;
  std::string absolutePath = "";
  for (std::list<std::string>::iterator i = files.begin (); i != files.end () && j < nodeContainerSize; i++, j++)
    {
      absolutePath = dirPath + "/" + (*i);
      ChooseRoute (nodeContainer.Get (j), absolutePath);
    }
  return 0;
}
std::list<std::string>
RoutesMobilityHelper::Open (std::string dirPath)
{
  NS_LOG_FUNCTION (this);
  DIR* dir;
  dirent* pdir;
  std::list<std::string> files;
  std::string fname = "";
  dir = opendir (dirPath.c_str ());

  while ((pdir = readdir (dir)))
    {
      if (!((strcmp (pdir->d_name, ".") == 0) || (strcmp (pdir->d_name,"..") == 0)))
        {
          fname = pdir->d_name;
          if (fname.find ("xml", fname.length () - 3) != std::string::npos)
            {
              files.push_back (fname);
            }
        }
    }

  return files;
}
int RoutesMobilityHelper::ChooseRoute (Ptr<Node> node, std::string start, std::string end, std::string redirectedDestination, double timeToTrigger)
{
  NS_LOG_FUNCTION (this);
  std::list<Ptr<Step> > stepList;
  std::list<Ptr<Point> > pointList;
  Time lastTime;
  Ptr<Point> p, pt;
  bool isRedirected = false;
  std::ostringstream strs;
  std::list<Ptr<Leg> > legList, auxLegList;
  double lastPointTime;
  legList = m_strategy->PerformRequest (start, end, m_travelMode, m_departureTime);
  for (std::list<Ptr<Leg> >::iterator i = legList.begin (); i != legList.end (); i++)
    {
      (*i)->GetStepList (stepList);
      for (std::list<Ptr<Step> >::iterator j = stepList.begin (); j != stepList.end () && !isRedirected; j++)
        {
          pointList = (*j)->GetPointList ();
          for (std::list<Ptr<Point> >::iterator k = pointList.begin (); k != pointList.end () && !isRedirected; k++)
            {
              if ((*k)->GetWaypointTime () > timeToTrigger)
                {
                  p = ((*k));
                  lastTime = Time (p->GetWaypointTime ());
                  pointList.erase (k, pointList.end ());
                  strs.str ("");
                  strs.clear ();
                  strs << p->GetLatitude ();
                  strs << ",";
                  strs << p->GetLongitude ();
                  auxLegList = m_strategy->PerformRequest (strs.str (), redirectedDestination, m_travelMode, m_departureTime);
                  isRedirected = true;
                }
            }
          if (isRedirected)
            {
              stepList.erase (j,stepList.end ());
            }
        }
      (*i)->SetStepList (stepList);
    }
  for (std::list<Ptr<Leg> >::iterator i = auxLegList.begin (); i != auxLegList.end (); i++)
    {
      (*i)->GetStepList (stepList);
      for (std::list<Ptr<Step> >::iterator j = stepList.begin (); j != stepList.end (); j++)
        {
          pointList = (*j)->GetPointList ();
          for (std::list<Ptr<Point> >::iterator k = pointList.begin (); k != pointList.end (); k++)
            {
              pt = ((*k));
              lastPointTime = lastTime.GetSeconds ();
              lastPointTime += ((p->GetWaypointTime () + pt->GetWaypointTime ()) + 0.001);
              pt->SetWaypointTime (lastPointTime);
            }
        }
    }
  for (std::list<Ptr<Leg> >::iterator i = auxLegList.begin (); i != auxLegList.end (); i++)
    {
      legList.push_back ((*i));
    }
  SchedulePoints (node, legList);
  return 0;
}

void
RoutesMobilityHelper::SetTransportationMethod (std::string travelMode)
{
  NS_LOG_FUNCTION (this);
  this->m_travelMode = travelMode;
}
void
RoutesMobilityHelper::SetDepartureTime (std::string departureTime)
{
  NS_LOG_FUNCTION (this);
  this->m_departureTime = departureTime;
}
int
RoutesMobilityHelper::ChooseRoute (NodeContainer& nodes, double upperLat, double upperLng, double lowerLat, double lowerLng)
{
  unsigned int numOfNodes = nodes.GetN ();
  double startLat = 0, startLng = 0, destLat = 0, destLng = 0;
  std::string start = "", end = "";
  double random = 0;
  for (unsigned int i = 0; i < numOfNodes; i++)
    {
      random = GenerateRandomValue (0,1);
      startLat = lowerLat + (upperLat - lowerLat) * random;
      random = GenerateRandomValue (0,1);
      startLng = lowerLng + (upperLng - lowerLng) * random;
      random = GenerateRandomValue (0,1);
      destLat = lowerLat + (upperLat - lowerLat) * random;
      random = GenerateRandomValue (0,1);
      destLng = lowerLng + (upperLng - lowerLng) * random;
      start = LocationToString (startLat,startLng);
      end = LocationToString (destLat, destLng);
      ChooseRoute (start, end, nodes.Get (i));
    }
  return 0;
}

int
RoutesMobilityHelper::ChooseRoute (NodeContainer& nodes, double startUpperLat, double startUpperLng, double startLowerLat, double startLowerLng,
                                   double destUpperLat, double destUpperLng, double destLowerLat, double destLowerLng)
{
  unsigned int numOfNodes = nodes.GetN ();
  double startLat = 0, startLng = 0, destLat = 0, destLng = 0;
  std::string start = "", end = "";
  double random = 0;
  for (unsigned int i = 0; i < numOfNodes; i++)
    {
      random = GenerateRandomValue (0,1);
      startLat = startLowerLat + (startUpperLat - startLowerLat) * random;
      random = GenerateRandomValue (0,1);
      startLng = startLowerLng + (startUpperLng - startLowerLng) * random;
      random = GenerateRandomValue (0,1);
      destLat = destLowerLat + (destUpperLat - destLowerLat) * random;
      random = GenerateRandomValue (0,1);
      destLng = destLowerLng + (destUpperLng - destLowerLng) * random;
      start = LocationToString (startLat, startLng);
      end = LocationToString (destLat, destLng);
      ChooseRoute (start, end, nodes.Get (i));
    }
  return 0;
}


double
RoutesMobilityHelper::GenerateRandomValue (double min, double max)
{
  double number = (double) rand () / RAND_MAX;
  double randNum = 0;
  randNum = min + number * (max - min);
  return randNum;
}


std::string
RoutesMobilityHelper::LocationToString (double lat, double lng)
{
  std::ostringstream strs;
  strs.precision (10);
  strs << lat;
  strs << ",";
  strs << lng;
  return strs.str ();
}


} //namespace ns3
