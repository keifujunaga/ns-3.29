/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Mohammed J.F. Alenazi
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
 * Author: Mohammed J.F. Alenazi  <malenazi@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported by King Saud University and
 * the ITTC at The University of Kansas.
 */

#include "epidemic-helper.h"
#include "ns3/epidemic-routing-protocol.h"
#include "ns3/epidemic-helper.h"

///original
#include "ns3/olsr-routing-protocol.h"
#include "ns3/olsr-helper.h"


/**
 * \file
 * \ingroup epidemic
 * ns3::EpidemicHelper implementation.
 */

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EpidemicHelper");

EpidemicHelper::~EpidemicHelper ()
{
  NS_LOG_FUNCTION (this);
}

EpidemicHelper::EpidemicHelper () : Ipv4RoutingHelper ()
{
  NS_LOG_FUNCTION (this);
  m_agentFactory.SetTypeId ("ns3::Epidemic::RoutingProtocol");
}

EpidemicHelper* EpidemicHelper::Copy (void) const
{
  NS_LOG_FUNCTION (this);
  return new EpidemicHelper (*this);
}

Ptr<Ipv4RoutingProtocol> EpidemicHelper::Create (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);
  Ptr<Epidemic::RoutingProtocol>
  agent = m_agentFactory.Create<Epidemic::RoutingProtocol> ();
  node->AggregateObject (agent);
  return agent;
}

void EpidemicHelper::Set (std::string name, const AttributeValue &value)
{
  NS_LOG_FUNCTION (this << name);
  m_agentFactory.Set (name, value);
}

void EpidemicHelper::Drop(NodeContainer c, int uid)
{
  int count = 0;
  NodeContainer::Iterator i;
  for(i = c.Begin(); i != c.End(); ++i)
  {
    Ptr<Node> node = c.Get (count);
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
    Ptr<Epidemic::RoutingProtocol> epi = DynamicCast<Epidemic::RoutingProtocol> (proto);
    NS_LOG_INFO("(MyDrop) node is " << (*i)->GetId());
    epi->MyDrop(uid);
    count++;
  }
}

void EpidemicHelper::SendJudge(NodeContainer c, int snodenum, Ptr<const Packet> packet, bool judge)
{ 
  Ptr<Node> node = c.Get(snodenum);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
  Ptr<Epidemic::RoutingProtocol> epi = DynamicCast<Epidemic::RoutingProtocol> (proto);
  epi->SendJudge(packet, judge);
}

void EpidemicHelper::setOpp_preds(NodeContainer c, int snodenum, std::map<Ipv4Address, double> main_preds)
{
  Ptr<Node> node = c.Get(snodenum);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
  Ptr<Epidemic::RoutingProtocol> epi = DynamicCast<Epidemic::RoutingProtocol> (proto);
  epi->setOpp_preds(main_preds);
}

void EpidemicHelper::setOpp_preds2(NodeContainer c, int snodenum, std::map<Ipv4Address, double> main_preds)
{
  Ptr<Node> node = c.Get(snodenum);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
  Ptr<Epidemic::RoutingProtocol> epi = DynamicCast<Epidemic::RoutingProtocol> (proto);
  epi->setOpp_preds2(main_preds);
}

} //end namespace ns3
