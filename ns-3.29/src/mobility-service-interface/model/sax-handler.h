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
#ifndef SAXHANDLER_H
#define SAXHANDLER_H

#include <xercesc/sax2/DefaultHandler.hpp>
#include "leg.h"
#include "step.h"

XERCES_CPP_NAMESPACE_USE
namespace ns3 {
/**
 * \ingroup mobility
 *
 * \brief The SAX-based parser for the Google Maps Directions API's XML response
 */
class SaxHandler : public xercesc::DefaultHandler
{
public:
  SaxHandler ( LegList& legList);
//    SaxHandler(const SaxHandler& orig);
  virtual ~SaxHandler ();

  void startElement (
    const   XMLCh* const    uri,
    const   XMLCh* const    localname,
    const   XMLCh* const    qname,
    const  xercesc::Attributes&     attrs
    );
  void endElement (
    const XMLCh* const uri,
    const XMLCh* const localname,
    const XMLCh* const qname
    );

  void characters (const XMLCh* const chars,const XMLSize_t length);
  void fatalError (const xercesc::SAXParseException&);
private:
  std::list<Ptr<Step> > stepList_;
  LegList& legList_;
  int durationOfStep;
  std::string polyline, status;
  bool foundStep,foundLeg,foundPolyline, foundDurationValue, foundDuration, foundStatus;
};
}
#endif  /* SAXHANDLER_H */

