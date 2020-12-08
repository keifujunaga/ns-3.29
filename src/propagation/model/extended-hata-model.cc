/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011, 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Marco Miozzo  <marco.miozzo@cttc.es>,
 *         Nicola Baldo <nbaldo@cttc.es>
 * 
 */
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/mobility-model.h"

#include <cmath>

#include <algorithm>
#include <cmath>

#include "extended-hata-model.h"
#include "ns3/core-module.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ExtendedHataModel");

NS_OBJECT_ENSURE_REGISTERED (ExtendedHataModel);

TypeId
ExtendedHataModel::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::ExtendedHataModel")
          .SetParent<PropagationLossModel> ()
          .SetGroupName ("Propagation")
          .AddConstructor<ExtendedHataModel> ()
          .AddAttribute ("Frequency", "The propagation frequency in Hz", DoubleValue (2400e6),
                         MakeDoubleAccessor (&ExtendedHataModel::m_frequency),
                         MakeDoubleChecker<double> ())
          .AddAttribute ("Environment", "Environment Scenario", EnumValue (UrbanEnvironment),
                         MakeEnumAccessor (&ExtendedHataModel::m_environment),
                         MakeEnumChecker (UrbanEnvironment, "Urban", SubUrbanEnvironment,
                                          "SubUrban", OpenAreasEnvironment, "OpenAreas"))
          .AddAttribute (
              "CitySize", "Dimension of the city", EnumValue (LargeCity),
              MakeEnumAccessor (&ExtendedHataModel::m_citySize),
              MakeEnumChecker (SmallCity, "Small", MediumCity, "Medium", LargeCity, "Large"));
  return tid;
}

ExtendedHataModel::ExtendedHataModel () : PropagationLossModel ()
{
  std::cout << "Do ExtendedHataModel" << std::endl;
}

ExtendedHataModel::~ExtendedHataModel ()
{
}

double a(double frequency, double Hm){
    return (1.1 * std::log10(frequency) - 0.7) * std::min(10.0, Hm) - (1.56 * std::log10(frequency) - 0.8) + std::max(0.0, 20 * std::log10(Hm / 10));
}

double b(double Hb){
    return std::min(0.0, 20 * std::log10(Hb / 30.0));
}

double orimax (double a, double b){
    if (a >= b) {
        return a;
    } else {
        return b;
    }
}

double orimin (double a, double b){
    if (a <= b) {
        return a;
    } else {
        return b;
    }
}

double PLrange004(double frequency, double distance, double Hb, double Hm){
    return 32.4 + 20 * std::log10(frequency) + 10 * std::log10(std::pow(distance,2) + std::pow((Hb - Hm),2) / std::pow(10,6));
}

double PLrange01(double frequency, double distance, double Hb, double Hm, double alpha){
    if (150 < frequency && frequency <= 1500){
        return 69.6 + 26.2 * std::log10(frequency) - 13.82 * std::log10(std::max(30.0,Hb)) + 
        (44.9 - 6.55 * std::log10(std::max(30.0,Hb))) * std::pow(std::log10(distance), alpha) - a(frequency, Hm) - b(Hb);
    }

    if (1500 < frequency && frequency <= 2000){
        return 46.3 + 33.9 * std::log10(frequency) - 13.82 * std::log10(std::max(30.0,Hb)) + 
        (44.9 - 6.55 * std::log10(std::max(30.0,Hb))) * std::pow(std::log10(distance), alpha) - a(frequency, Hm) - b(Hb);
    }

    if (2000 < frequency && frequency <= 3000){
        return 46.3 + 33.9 * std::log10(2000) + 10 * std::log10(frequency / 2000) - 13.82 * std::log10(std::max(30.0,Hb)) + 
        (44.9 - 6.55 * std::log10(std::max(30.0,Hb))) * std::pow(std::log10(distance), alpha) - a(frequency, Hm) - b(Hb);
    }
    std::cout << "error is occured" << std::endl; return -1;
}

double PLrange004to01(double frequency, double distance, double Hb, double Hm, double alpha){
    return PLrange004(frequency, distance, Hb, Hm) + 
    (std::abs(std::log10(distance) - std::log10(0.04))) / (std::abs(std::log10(0.1) - std::log10(0.04))) * 
    std::abs(PLrange01(frequency, distance, Hb, Hm, alpha) - PLrange004(frequency, distance, Hb, Hm));
}

double PLrange01_suburban (double frequency, double distance, double Hb, double Hm, double alpha){
    return PLrange01 (frequency, distance, Hb, Hm, alpha) - 2 * pow(log10(orimin(orimax(150, frequency), 2000)/28),2) - 5.4;
}

double StandardDeviation(double distance){
    if (distance <= 0.04)
        return 3.5;
    else if (0.04 < distance && distance <= 0.1)
        return 3.5 + (12 - 3.5) / (0.1 - 0.04) * (distance - 0.04);
    else if (0.1 < distance && distance <= 0.2)
        return 12;
    else if (0.2 < distance && distance <= 0.6)
        return 12 + (9 - 12) / (0.6 - 0.2) * (distance - 0.2);
    else if (0.6 < distance)
        return 9;
    std::cout << "error is occured" << std::endl; return -1;
}

double
ExtendedHataModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  double alpha = 1;
  double fmhz = m_frequency / 1e6;
  double dist = a->GetDistanceFrom (b) / 1000.0;
  // double ans;
  //std::cout << "fmhz is " << fmhz << std::endl;
  // std::cout << "Distance(km) is " << dist << " at " << Simulator::Now().GetSeconds() << "s" << std::endl;
  double hb =
      (a->GetPosition ().z > b->GetPosition ().z ? a->GetPosition ().z : b->GetPosition ().z);
  double hm =
      (a->GetPosition ().z < b->GetPosition ().z ? a->GetPosition ().z : b->GetPosition ().z);

  if (hb == 200 && hm == 200) 
  {
    //std::cout << "using friss propagation model" << std::endl;
    double lamda = 300 / fmhz;
    return 20 * log10((4 * M_PI * dist*1000) / lamda);
  }

  if (dist <= 0.04)
    return PLrange004(fmhz, dist, hb, hm);
    //return PLrange004 (fmhz, dist, hb, hm);
  else if (0.04 < dist && dist < 0.1)
    return PLrange004to01(fmhz, dist, hb, hm, alpha);
    // return PLrange004to01 (fmhz, dist, hb, hm, alpha);
  else if (0.1 <= dist)
    return PLrange01_suburban(fmhz, dist, hb, hm, alpha);
    //return PLrange01 (fmhz, dist, hb, hm, alpha);
  std::cout << "error is occured" << std::endl; return -1;
}

double
ExtendedHataModel::DoCalcRxPower (double txPowerDbm, Ptr<MobilityModel> a,
                                  Ptr<MobilityModel> b) const
{
  //std::cout << "txPowerDbm is " << txPowerDbm << std::endl;
  //std::cout << a->GetDistanceFrom(b) << std::endl;
  double getloss = txPowerDbm - GetLoss(a, b);
  //std::cout << "Getloss value is " << getloss << " at "<< Simulator::Now().GetSeconds() << "s" << std::endl;
  return getloss;
}

int64_t
ExtendedHataModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

} // namespace ns3
