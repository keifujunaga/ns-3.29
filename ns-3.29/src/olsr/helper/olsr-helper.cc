/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "olsr-helper.h"
#include "ns3/olsr-routing-protocol.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3 {

OlsrHelper::OlsrHelper ()
{
  m_agentFactory.SetTypeId ("ns3::olsr::RoutingProtocol");
}

OlsrHelper::OlsrHelper (const OlsrHelper &o) : m_agentFactory (o.m_agentFactory)
{
  m_interfaceExclusions = o.m_interfaceExclusions;
}

OlsrHelper *
OlsrHelper::Copy (void) const
{
  return new OlsrHelper (*this);
}

void
OlsrHelper::ExcludeInterface (Ptr<Node> node, uint32_t interface)
{
  std::map<Ptr<Node>, std::set<uint32_t>>::iterator it = m_interfaceExclusions.find (node);

  if (it == m_interfaceExclusions.end ())
    {
      std::set<uint32_t> interfaces;
      interfaces.insert (interface);

      m_interfaceExclusions.insert (std::make_pair (node, std::set<uint32_t> (interfaces)));
    }
  else
    {
      it->second.insert (interface);
    }
}

Ptr<Ipv4RoutingProtocol>
OlsrHelper::Create (Ptr<Node> node) const
{
  Ptr<olsr::RoutingProtocol> agent = m_agentFactory.Create<olsr::RoutingProtocol> ();

  std::map<Ptr<Node>, std::set<uint32_t>>::const_iterator it = m_interfaceExclusions.find (node);

  if (it != m_interfaceExclusions.end ())
    {
      agent->SetInterfaceExclusions (it->second);
    }

  node->AggregateObject (agent);
  return agent;
}

void
OlsrHelper::Set (std::string name, const AttributeValue &value)
{
  m_agentFactory.Set (name, value);
}

int64_t
OlsrHelper::AssignStreams (NodeContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<Node> node;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      node = (*i);
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      NS_ASSERT_MSG (ipv4, "Ipv4 not installed on node");
      Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
      NS_ASSERT_MSG (proto, "Ipv4 routing not installed on node");
      Ptr<olsr::RoutingProtocol> olsr = DynamicCast<olsr::RoutingProtocol> (proto);
      if (olsr)
        {
          currentStream += olsr->AssignStreams (currentStream);
          continue;
        }
      // Olsr may also be in a list
      Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (proto);
      if (list)
        {
          int16_t priority;
          Ptr<Ipv4RoutingProtocol> listProto;
          Ptr<olsr::RoutingProtocol> listOlsr;
          for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
            {
              listProto = list->GetRoutingProtocol (i, priority);
              listOlsr = DynamicCast<olsr::RoutingProtocol> (listProto);
              if (listOlsr)
                {
                  currentStream += listOlsr->AssignStreams (currentStream);
                  break;
                }
            }
        }
    }
  return (currentStream - stream);
}

void
OlsrHelper::aaa (Ptr<Node> node, Ipv4Address addresses[])
{
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); 
  NS_ASSERT_MSG (ipv4, "Ipv4 not installed on node");
  Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
  NS_ASSERT_MSG (proto, "Ipv4 routing not installed on node");
  Ptr<olsr::RoutingProtocol> olsr = DynamicCast<olsr::RoutingProtocol> (proto);

  if (olsr)
    {
      std::vector<olsr::RoutingTableEntry> table = olsr->GetRoutingTableEntries ();
      for (uint32_t i = 0; i < table.size (); ++i)
        {
          olsr::RoutingTableEntry route = table[i];
          std::cout << "dest is " << route.destAddr << ", pred is " << route.pred << ", interface is " << route.interface << std::endl;
          addresses[i] = route.destAddr;
        }
    }
  // Olsr may also be in a list
  Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (proto);
  if (list)
    {
      int16_t priority;
      Ptr<Ipv4RoutingProtocol> listProto;
      Ptr<olsr::RoutingProtocol> listOlsr;
      for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
        {
          listProto = list->GetRoutingProtocol (i, priority);
          listOlsr = DynamicCast<olsr::RoutingProtocol> (listProto);
          if (listOlsr)
            {
              std::vector<olsr::RoutingTableEntry> table2 = listOlsr->GetRoutingTableEntries ();
              for (uint32_t i = 0; i < table2.size (); ++i)
                {
                  olsr::RoutingTableEntry route2 = table2[i];
                  //std::cout << "dest is " << route2.destAddr << ", pred is " << route2.pred << ", interface is " << route2.interface << std::endl;
                  addresses[i] = route2.destAddr;
                }
            }
        }
    }
}

void
OlsrHelper::aaa2(Ptr<Node> node, Ipv4Address addresses[])
{
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); 
  NS_ASSERT_MSG (ipv4, "Ipv4 not installed on node");
  Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
  NS_ASSERT_MSG (proto, "Ipv4 routing not installed on node");
  Ptr<olsr::RoutingProtocol> olsr = DynamicCast<olsr::RoutingProtocol> (proto);

  if (olsr)
    {
      std::vector<olsr::RoutingTableEntry> table = olsr->GetRoutingTableEntries ();
      for (uint32_t i = 0; i < table.size (); ++i)
        {
          olsr::RoutingTableEntry route = table[i];
          // std::cout << "dest is " << route.destAddr << std::endl;
          addresses[i] = route.destAddr;
        }
    }
  // Olsr may also be in a list
  Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (proto);
  if (list)
    {
      int16_t priority;
      Ptr<Ipv4RoutingProtocol> listProto;
      Ptr<olsr::RoutingProtocol> listOlsr;
      for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
        {
          listProto = list->GetRoutingProtocol (i, priority);
          listOlsr = DynamicCast<olsr::RoutingProtocol> (listProto);
          if (listOlsr)
            {
              std::vector<olsr::RoutingTableEntry> table2 = listOlsr->GetRoutingTableEntries ();
              for (uint32_t i = 0; i < table2.size (); ++i)
                {
                  olsr::RoutingTableEntry route2 = table2[i];
                  // std::cout << "dest is " << route2.destAddr << std::endl;
                  addresses[i] = route2.destAddr;
                }
            }
        }
    }
}

void
OlsrHelper::ExtractIpPred (Ptr<Node> node, std::map <Ipv4Address, double> ipandpred)
{
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); 
  NS_ASSERT_MSG (ipv4, "Ipv4 not installed on node");
  Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
  NS_ASSERT_MSG (proto, "Ipv4 routing not installed on node");
  Ptr<olsr::RoutingProtocol> olsr = DynamicCast<olsr::RoutingProtocol> (proto);

  if (olsr)
    {
      std::vector<olsr::RoutingTableEntry> table = olsr->GetRoutingTableEntries ();
      for (uint32_t i = 0; i < table.size (); ++i)
        {
          olsr::RoutingTableEntry route = table[i];
          //std::cout << "dest is " << route.destAddr << ", pred is " << route.pred << ", interface is " << route.interface << std::endl;
          ipandpred[route.destAddr] = route.pred;
        }
    }
  // Olsr may also be in a list
  Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (proto);
  if (list)
    {
      int16_t priority;
      Ptr<Ipv4RoutingProtocol> listProto;
      Ptr<olsr::RoutingProtocol> listOlsr;
      for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
        {
          listProto = list->GetRoutingProtocol (i, priority);
          listOlsr = DynamicCast<olsr::RoutingProtocol> (listProto);
          if (listOlsr)
            {
              std::vector<olsr::RoutingTableEntry> table2 = listOlsr->GetRoutingTableEntries ();
              for (uint32_t i = 0; i < table2.size (); ++i)
                {
                  olsr::RoutingTableEntry route2 = table2[i];
                  /*
                  std::cout << "dest is " << route2.destAddr << ", pred is " << route2.pred << ", interface is " << route2.interface << std::endl;
                  */
                  ipandpred[route2.destAddr] = route2.pred;
                  std::cout << "ipandpred[" << route2.destAddr << " = route2.pred = " << ipandpred[route2.destAddr] << std::endl; 
                }
            }
        }
    }
}

void 
OlsrHelper::setpred(NodeContainer c, int snodenum, double pred)
{
  Ptr<Node> node = c.Get(snodenum);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
  Ptr<olsr::RoutingProtocol> olsr = DynamicCast<olsr::RoutingProtocol> (proto);
  olsr->setpred(pred);
}
} // namespace ns3
