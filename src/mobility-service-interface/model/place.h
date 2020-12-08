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

#ifndef PLACE_H
#define PLACE_H
#include <ns3/object.h>
#include <ns3/log.h>
namespace ns3 {
/**
 * \ingroup mobility
 *
 * \brief This class represents a Google Maps Place
 *
 * This class is a representation of a real-world Google Maps Place, such as restaurants, hotels, gas stations, etc.
 * This is used to provide automatic start and endpoints to nodes
 */
class Place : public ns3::Object
{
public:
  /**
   * \brief Creates a Place object
   *
   * Creates a Place object, initializing its values to 0
   */
  Place ();
  /**
   * \brief Creates a Place object
   *
   * \param lat The latitude of the Place
   * \param lng The longitude of the Place
   */
  Place (double lat, double lng);
  /**
   * \brief Creates a Place object
   *
   * Standard copy constructor
   *
   * \param orig The object to copy
   */
  Place (const Place& orig);
  virtual ~Place ();
  /**
   * \brief Get the latitude of the Place
   *
   * \return m_lat A double that represents the latitude of the place
   */
  double GetLatitude ();
  /**
   * \brief Get the longitude of the Place
   * \return m_lng A double that represents the longitude of the place
   */
  double GetLongitude ();
  /**
   * \brief Set the next page token
   *
   * If the request contains more than 20 Places, the Google Maps Places service divides the total result into pages. To access them, an attribute called pagetoken is provided. If present, it indicates that there are more results than those shown.
   * \param nextPageToken
   */
  void SetNextPageToken (std::string nextPageToken);
  /**
   * \brief Get the next page token
   *
   * \return m_nextPageToken A std::string that contains the XML's pagetoken attribute
   */
  std::string GetNextPageToken ();
private:
  double m_lat, m_lng; //!< The latitude and longitude coordinates for the place, in WSG84 format
  std::string m_nextPageToken;

};
std::ostream& operator<< (std::ostream& out, Place &p);
typedef std::vector<Ptr<Place> > PlaceList;
} //namespace ns3
#endif  /* PLACE_H */

