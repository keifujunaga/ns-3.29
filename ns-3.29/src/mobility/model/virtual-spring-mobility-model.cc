/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015,2016 CONELAB UOU
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
 * Author: Md Mehedi Hasan <hasan3345@gmail.com>
 */

#include "virtual-spring-mobility-model.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("VirtualSpringMobilityModelMobility");

NS_OBJECT_ENSURE_REGISTERED (VirtualSpringMobilityModel);

TypeId
VirtualSpringMobilityModel::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::VirtualSpringMobilityModel")
          .SetParent<MobilityModel> ()
          .SetGroupName ("Mobility")
          .AddConstructor<VirtualSpringMobilityModel> ()
          .AddAttribute ("Radius", "A double value used to pick the radius of the way (m).",
                         DoubleValue (10.0),
                         MakeDoubleAccessor (&VirtualSpringMobilityModel::m_radius),
                         MakeDoubleChecker<double> ())
          .AddAttribute ("Speed", "A double value used to pick the speed (m/s).", DoubleValue (1.0),
                         MakeDoubleAccessor (&VirtualSpringMobilityModel::m_speed),
                         MakeDoubleChecker<double> ())
          .AddAttribute ("Theta", "A double value used to pick the theta.", 
                         DoubleValue (0.0),
                         MakeDoubleAccessor (&VirtualSpringMobilityModel::m_theta),
                         MakeDoubleChecker<double> ())
          .AddAttribute ("NodeVelocity", "A Vector value used to pick the speed (m/s).", 
                         VectorValue (Vector(0.0, 0.0, 200.0)),
                         MakeVectorAccessor (&VirtualSpringMobilityModel::m_velocity),
                         MakeVectorChecker ());
  return tid;
}

void
VirtualSpringMobilityModel::DoInitialize (void)
{
  DoInitializePrivate ();
  MobilityModel::DoInitialize ();
}

void
VirtualSpringMobilityModel::DoInitializePrivate (void)
{
  m_helper.Update ();
  // original 
  /* 
  double pi = 3.1416;
  double speed = m_speed;
  double radius = m_radius;
  double omega = speed / radius;
  double theta = GetTheta ();

  if ((theta + omega) < (2 * pi))
    {
      theta += omega;
      SetTheta (theta);
    }
  else
    {
      theta = (theta + omega) - (2 * pi);
      SetTheta (theta);
    }

  // 
  Vector vector ((radius * std::cos (theta)) * omega, (radius * std::sin (theta)) * omega, 0.0);
  */

  // 1. make function of velocity set m_velocity which is set Virtual Spring Model Velocity.
  Vector velocity = m_velocity;
  // NS_LOG_UNCOND("velocity is " << velocity);
  // NS_LOG_UNCOND("m_velocity is " << m_velocity);
  // NS_LOG_UNCOND("Simulator time: " << Simulator::Now());
  // m_helper.SetVelocity (vector);
  m_helper.SetVelocity (velocity);
  m_helper.Unpause ();
  // delayLeft:waypointの間隔を設定
  Time delayLeft = Seconds (1.0);
  DoWalk (delayLeft);
}

void
VirtualSpringMobilityModel::DoWalk (Time delayLeft)
{
  // NS_LOG_UNCOND("delayLeft.GetSeconds() " << delayLeft.GetSeconds());
  Vector position = m_helper.GetCurrentPosition ();
  Vector speed = m_helper.GetVelocity ();
  Vector nextPosition = position;
  nextPosition.x += speed.x * delayLeft.GetSeconds ();
  nextPosition.y += speed.y * delayLeft.GetSeconds ();
  m_event.Cancel ();
  m_event = Simulator::Schedule (delayLeft, &VirtualSpringMobilityModel::DoInitializePrivate, this);
  NotifyCourseChange ();
}

void
VirtualSpringMobilityModel::DoDispose (void)
{
  // chain up
  MobilityModel::DoDispose ();
}

Vector
VirtualSpringMobilityModel::DoGetPosition (void) const
{
  m_helper.Update ();
  return m_helper.GetCurrentPosition ();
}

void
VirtualSpringMobilityModel::DoSetPosition (const Vector &position)
{
  m_helper.SetPosition (position);
  Simulator::Remove (m_event);
  m_event = Simulator::ScheduleNow (&VirtualSpringMobilityModel::DoInitializePrivate, this);
}

Vector
VirtualSpringMobilityModel::DoGetVelocity (void) const
{
  return m_helper.GetVelocity ();
}

void
VirtualSpringMobilityModel::SetTheta (double &theta)
{
  m_theta = theta;
}

double
VirtualSpringMobilityModel::GetTheta (void) const
{
  return m_theta;
}

// 1. set additional code
void VirtualSpringMobilityModel::SetVSMVelocity (Vector velocity)
{
  m_velocity = velocity;
  NS_LOG_UNCOND("additional m_velocity is " << m_velocity);
  NS_LOG_UNCOND("Simulator time: " << Simulator::Now());
}
} // namespace ns3