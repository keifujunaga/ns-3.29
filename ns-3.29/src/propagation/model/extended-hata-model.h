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
 * Author: Marco Miozzo  <marco.miozzo@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 * 
 */

#ifndef EXTENDED_HATA_MODEL_H
#define EXTENDED_HATA_MODEL_H

#include <ns3/propagation-loss-model.h>
#include <ns3/propagation-environment.h>

namespace ns3 {


/**
 * \ingroup propagation
 *
 * \brief this class implements the Okumura Hata propagation loss model
 * 
 * this class implements the Okumura Hata propagation loss model,
 * which is used to model open area pathloss for distances > 1 Km
 * and frequencies ranging from 150 MHz to 2.0 GHz. 
 * For more information about the model, please see
 * the propagation module documentation in .rst format.
 */
class ExtendedHataModel : public PropagationLossModel
{

public:

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  ExtendedHataModel ();
  virtual ~ExtendedHataModel ();

  /** 
   * \param a the first mobility model
   * \param b the second mobility model
   * 
   * \return the loss in dBm for the propagation between
   * the two given mobility models
   */
  double GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

private:
  /**
   * \brief Copy constructor
   *
   * Defined and unimplemented to avoid misuse
   */
  ExtendedHataModel (const ExtendedHataModel &);
  /**
   * \brief Copy constructor
   *
   * Defined and unimplemented to avoid misuse
   * \returns
   */
  ExtendedHataModel & operator = (const ExtendedHataModel &);

  virtual double DoCalcRxPower (double txPowerDbm,
                                Ptr<MobilityModel> a,
                                Ptr<MobilityModel> b) const;
  virtual int64_t DoAssignStreams (int64_t stream);
  
  /*
  double a(double frequency, double Hm);
  double b(double Hb);
  double PLrange004(double frequency, double distance, double Hb, double Hm);
  double PLrange01(double frequency, double distance, double Hb, double Hm, double alpha);
  double PLrange004to01(double frequency, double distance, double Hb, double Hm, double alpha);
  double StandardDeviation(double distance);
  */
  
  EnvironmentType m_environment;  //!< Environment Scenario
  CitySize m_citySize;  //!< Size of the city
  double m_frequency; //!< frequency in Hz
};

} // namespace ns3


#endif // EXTENDED_HATA_MODEL_H
