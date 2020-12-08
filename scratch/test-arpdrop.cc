/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
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
 */

/*this script is used to used to characterize the throughput and delay of in an
adhoc 802.11 network for different packet sizes and data rates. Since the intent is
to create an adhoc network, there is no routing (or meshing as per my understanding) and
hence a node will be able to communicate with another node if it is in range.*/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

NS_LOG_COMPONENT_DEFINE ("arp-test");

using namespace ns3;


void arpDrop(Ptr <const Packet> pkt){std::cout<<"arp packet dropped"<<std::endl;}

int main (int argc, char *argv[])
{
  double distance = 2;  // m
  uint32_t numFlows = 5;  //
  uint32_t packetSize = 80; // bytes
  char appDataRate[128] = "64kbps";//kbps
  double stopTime = 10.0;
  double delayBinWidth = 0.0001;
  CommandLine cmd;

  cmd.AddValue ("distance", "distance (m)", distance);
  cmd.AddValue ("packetSize", "packet size in Bytes", packetSize);
  cmd.AddValue ("numFlows", "number of flows", numFlows);
  cmd.AddValue ("appDataRate", "appDataRate in kbps", appDataRate);
  cmd.AddValue ("stopTime", "stop time in seconds", stopTime);
  cmd.AddValue ("delayBinWidth", "flowmon histogram delay bin width", delayBinWidth);
  cmd.Parse (argc, argv);
 
  std::cout<<"Simulation Parameters ::" << "\n" <<
    "distance     = "<< distance    << "\n" <<
    "packetSize     = "<< packetSize<< "\n" <<
    "numFlows    = "<< numFlows     << "\n" <<
    "app Data Rate    = "<< appDataRate<<"\n" << std::endl;


  int numNodes = numFlows*2;

  NodeContainer c;
  c.Create (numNodes);
 
  //Config::Set("/NodeList/*/$ns3::ArpL3Protocol/CacheList/*/WaitReplyTimeout",TimeValue(Seconds(0.25)));
  //Config::Set("/NodeList/*/$ns3::ArpL3Protocol/CacheList/*/MaxRetries",UintegerValue(100));

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                "DataMode", StringValue("DsssRate2Mbps"),
                "RtsCtsThreshold",UintegerValue(2200));

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
 
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  wifiPhy.Set("TxPowerStart",DoubleValue(0));
  wifiPhy.Set("TxPowerEnd",DoubleValue(0));
  wifiPhy.Set("TxGain",DoubleValue(0));
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a non-QoS upper mac, and disable rate control
   WifiMacHelper wifiMac;
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);
 
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                "X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=4.0]"),
                "Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=4.0]"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);

 
  InternetStackHelper internet;
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ifcont = ipv4.Assign (devices);

  // Create Apps
 
 
  Address* sinkAddress[numNodes];
  OnOffHelper* onOff[numNodes];
  ApplicationContainer onOffApp[numNodes];

  PacketSinkHelper* packetSinkHelper[numNodes];
  ApplicationContainer sinkApApp[numNodes];

  uint16_t portAddr=9;
  uint32_t sinkNodeNum;
 
  for( uint16_t ii =0; ii<numFlows; ii=ii+1){
      sinkNodeNum = ii+numFlows;
      std::cout<<ii<<"\t"<<sinkNodeNum<<std::endl;

      sinkAddress[ii] = new Address(InetSocketAddress(ifcont.GetAddress(sinkNodeNum),portAddr));

      //Install packet sink on node ii
      packetSinkHelper[ii] = new PacketSinkHelper("ns3::UdpSocketFactory", *(sinkAddress[ii]));
      sinkApApp[ii] = (*packetSinkHelper[ii]).Install(c.Get(sinkNodeNum));
      sinkApApp[ii].Start(Seconds(1.0));
      sinkApApp[ii].Stop(Seconds(100.0));


      // Traffic generators
      //Install onOff packet generator on node 0
      onOff[ii] = new OnOffHelper("ns3::UdpSocketFactory",(*sinkAddress[ii]));
      (*onOff[ii]).SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
      (*onOff[ii]).SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
      (*onOff[ii]).SetAttribute("DataRate", StringValue(appDataRate));
      (*onOff[ii]).SetAttribute("PacketSize", UintegerValue(packetSize));
     
      onOffApp[ii] = (*onOff[ii]).Install(c.Get(ii));
     
      (onOffApp[ii]).Start(Seconds(0.0));
      (onOffApp[ii]).Stop(Seconds(stopTime));
  }



  Simulator::Stop (Seconds (stopTime));




 
  //Config::ConnectWithoutContext("/NodeList/*/$ns3::ArpL3Protocol/CacheList/*/Drop", MakeCallback(&arpDrop)); 
  //Config::ConnectWithoutContext("/NodeList/*/$ns3::ArpL3Protocol/Drop", MakeCallback(&arpDrop)); 
  Config::ConnectWithoutContext("/NodeList/*/$ns3::Ipv4L3Protocol/InterfaceList/*/ArpCache/Drop", MakeCallback(&arpDrop)); 


  FlowMonitorHelper flowmon;
  flowmon.SetMonitorAttribute("DelayBinWidth",DoubleValue(delayBinWidth));
  flowmon.SetMonitorAttribute("StartTime",TimeValue(Seconds(5.0)));
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();
 

  Simulator::Run ();

  monitor->CheckForLostPackets();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
 
  std::cout<<"\n"<<"flowmon stats"<<"\n"<<std::endl;
  char destAddrFlowMon[32];
  sprintf(destAddrFlowMon,"10.1.1.%d",numNodes);
  Histogram jitterHist;
  //std::ostream os1;
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
  {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
    if(((t.sourceAddress == Ipv4Address("10.1.1.1")) && (t.destinationAddress == Ipv4Address(destAddrFlowMon))) || 1)
    {
    std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
    std::cout << "  Tx Pkts:   " << i->second.txPackets << "\n";
    std::cout << "  Rx Pkts:   " << i->second.rxPackets << "\n";
    std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
    std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
    std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds())/ 1024  << " kbps\n";
    std::cout << " Mean Delay:  " << i->second.delaySum.GetSeconds() / i->second.rxPackets << std::endl;
    std::cout << " Mean Jitter: " << i->second.jitterSum.GetSeconds()/ i->second.rxPackets << std::endl;

    }
     
  }

  Simulator::Destroy ();
  return 0;
}