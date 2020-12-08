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

#ifndef PLACESAPICONNECT_H
#define PLACESAPICONNECT_H
#include "ns3/place.h"
namespace ns3 {
/**
 * \ingroup mobility
 *
 * \brief An interface to the API used to request the location of Places
 */
class PlacesApiConnect
{
public:
  /**
   * \brief Performs a request to the Google Maps Places service
   *
   * This function is abstract and needs to be implemented
   */
  virtual std::vector<Ptr<Place> > PerformRequest (double lat, double lng, double radius) = 0;
private:
};
} //namespace ns3
#endif  /* PLACESAPICONNECT_H */

