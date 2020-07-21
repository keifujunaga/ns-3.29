#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/routes-mobility-helper.h"
#include "ns3/config-store.h"
#include "ns3/trace-helper.h"

#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/stats-module.h"
#include "ns3/point-to-point-module.h"
#include <ns3/buildings-module.h>
#include "ns3/applications-module.h"

#include <boost/algorithm/string.hpp>
#include <string>
#include <list>
#include <iostream>
#include <boost/foreach.hpp>

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("dicomo");

AnimationInterface * pAnim = 0;

// A
// int nodeappcount[23]={0,0,0,1,0,
//                       0,1,0,0,0,
//                       0,1,1,1,0,
//                       0,0,0,1,0,
//                       1,0,0};

// B
// int nodeappcount[23]={0,1,0,0,1,
//                       1,0,1,0,1,
//                       0,0,0,0,0,
//                       0,1,1,0,1,
//                       0,1,1};

// C
int nodeappcount[23]={0,0,0,0,0,
                      0,0,0,0,0,
                      0,0,0,0,0,
                      0,1,1,0,1,
                      0,1,1};


//Node create
NodeContainer ueNodes;
// setup onoff
uint16_t port = 50000;
OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port)));
ApplicationContainer app;

void ReceivedPacket(std::string context, Ptr<const Packet> packet ,const Address &)
{
   std::string delim ("/");
   std::list<std::string> list_string;
   boost::split(list_string, context, boost::is_any_of(delim));
   auto itr = list_string.begin();
   ++itr;
   ++itr;
   int kym = std::stoi(*itr); // NodeNo

   if(nodeappcount[kym] == 0){
     //NS_LOG_DEBUG(context); 
     NS_LOG_DEBUG( "Node " << *itr << " Received one Packet At time " << Simulator::Now ().GetSeconds () );
     app = onoff.Install (ueNodes.Get(kym));
     nodeappcount[kym] = 1;
   }
} 

  int main (int argc, char** argv){
  int numberOfueNodes  = 15;
  // int numberOfueNodes_2 =  2 * numberOfueNodes;
  bool verbose = false;
  double centerLat = 39.380000, centerLng = 141.955000, centerAltitude = 0;

  std::string animFile = "dicomo-N23-ETWS+ProSe.xml";

  CommandLine cmd;
  cmd.AddValue ("verbose", "add verbose logging", verbose);
  cmd.AddValue ("centerLatitude", "set the latitude for the center of the Cartesian plane", centerLat);
  cmd.AddValue ("centerLongitude", "set the longitude for the center of the Cartesian plane", centerLng);
  cmd.AddValue ("centerAltitude", "set the longitude for the center of the Cartesian plane", centerAltitude);
  cmd.Parse (argc, argv);
  if (verbose)
    {
      LogComponentEnable ("GoogleMapsApiConnect", LOG_LEVEL_DEBUG);
      LogComponentEnable ("GoogleMapsDecoder", LOG_LEVEL_DEBUG);
      LogComponentEnable ("Leg", LOG_LEVEL_DEBUG);
      LogComponentEnable ("Place", LOG_LEVEL_DEBUG);
      LogComponentEnable ("Point", LOG_LEVEL_DEBUG);
      LogComponentEnable ("RoutesMobilityHelper", LOG_LEVEL_DEBUG);
      LogComponentEnable ("Step", LOG_LEVEL_DEBUG);
      LogComponentEnable ("WaypointMobilityModel", LOG_LEVEL_DEBUG);
    }
  LogComponentEnable ("dicomo", LOG_LEVEL_DEBUG);

  // Create nodes container
  
  ueNodes.Create (numberOfueNodes);
  // NodeContainer enbNodes;
  // enbNodes.Create (numberOfenbNodes);

  // Set the mobility helper
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::WaypointMobilityModel");
  // Install mobility helper on the nodes
  mobility.Install (ueNodes);
  // mobility.Install (enbNodes);

  // Instantiate Routes Mobility Helper
  RoutesMobilityHelper routes (centerLat,centerLng,centerAltitude);
  RoutesMobilityHelper routes2 (centerLat,centerLng,centerAltitude);

  // put ueNodes
  std::vector<std::string> startEndTokenList1;
  float shel_x[15], shel_y[15];
  shel_x[0] = 39.639195;//new_kouraku
  shel_y[0] = 141.947284;
  shel_x[1] = 39.640594;//oosaka_ousyou
  shel_y[1] = 141.947732;
  shel_x[2] = 39.641127;//kyatoru miyako
  shel_y[2] = 141.945983;
  shel_x[3] = 39.641502;//gotou hihuka
  shel_y[3] = 141.951732;
  shel_x[4] = 39.641684;//kaigosisetsu aozora
  shel_y[4] = 141.954243;
  shel_x[5] = 39.642800;//saigai koueijyutaku
  shel_y[5] = 141.954005;
  shel_x[6] = 39.643069;//miyakohoteru_sawadaya
  shel_y[6] = 141.952436;
  shel_x[7] = 39.637814;//miyako_koutougakkou
  shel_y[7] = 141.947428;
  shel_x[8] = 39.644508;//miyako_daiichibyouin
  shel_y[8] = 141.947121;
  shel_x[9] = 39.638134;//yokohama_hachimangu
  shel_y[9] = 141.943701;
  shel_x[10] = 39.642449;//tateai_kinrinkouen
  shel_y[10] = 141.942089;
  shel_x[11] = 39.644772;//haneguro_jinja
  shel_y[11] = 141.951814;
  shel_x[12] = 39.645465;//jyouanji
  shel_y[12] = 141.955520;
  shel_x[13] = 39.646898;//syoubou_honbu
  shel_y[13] = 141.946614;
  shel_x[14] = 39.647591;//tsutsujigaoka_kouen
  shel_y[14] = 141.943700;

  float star_x[15], star_y[15]; // start 
  star_x[0] = 39.644459;
  star_y[0] = 141.946460;
  star_x[1] = 39.643435;
  star_y[1] = 141.947535;
  star_x[2] = 39.643170;
  star_y[2] = 141.945547;
  star_x[3] = 39.643875; 
  star_y[3] = 141.950655; 
  star_x[4] = 39.642386; 
  star_y[4] = 141.945824;
  star_x[5] = 39.642634; 
  star_y[5] = 141.948667;
  star_x[6] = 39.643031;  
  star_y[6] = 141.952926;
  star_x[7] = 39.642023;
  star_y[7] = 141.952218;
  star_x[8] = 39.641792;
  star_y[8] = 141.950394;
  star_x[9] = 39.641552;
  star_y[9] = 141.946875;
  star_x[10] = 39.638626;
  star_y[10] = 141.945666;
  star_x[11] = 39.638568;
  star_y[11] = 141.948134;
  star_x[12] = 39.638070;
  star_y[12] = 141.949545;
  star_x[13] = 39.635880; 
  star_y[13] = 141.947206;
  star_x[14] = 39.637359;  
  star_y[14] = 141.950607;

  int shel_c = 0;
  for(int i = 0; i < numberOfueNodes; i++){
    // if(i!=0 && i%5==0) shel_c++; // kokoga okasii
    if(i < 3) {
      shel_c = 8;
    } else if (i == 3){
      shel_c = 11;
    } else if (i == 4){
      shel_c = 2;
    } else if (i == 5){
      shel_c = 1;
    } else if (i == 6){
      shel_c = 6;
    } else if (i == 7 || i == 8){
      shel_c = 4; 
    } else if (i == 9){
      shel_c = 3;
    } else if (i == 10 || i == 11){
      shel_c = 0;
    } else if (i == 12 || i == 13 || i == 14){
      shel_c = 7;
    }
    char goal[100], start[100];
    snprintf(goal, 100,"%.6f, %.6f", shel_x[shel_c], shel_y[shel_c]);
    snprintf(start, 100,"%.6f, %.6f", star_x[i], star_y[i]);
    startEndTokenList1.push_back(start);
    startEndTokenList1.push_back(goal);
  }

  /*
  std::string startEndTokenList2[numberOfueNodes_2];
  for(int i = 0; i < numberOfueNodes_2; i++){
    startEndTokenList2[i] = startEndTokenList1[i];
  }
  */

  NodeContainer ueNode2;
  NodeContainer ueNode3;

  ueNode2.Add(ueNodes.Get(0));
  ueNode2.Add(ueNodes.Get(1));

  ueNode3.Add(ueNodes.Get(2));
  ueNode3.Add(ueNodes.Get(3));

  std::string startEndTokenList2[2];
  for(int i = 0; i < 2; i++){
    startEndTokenList2[i] = startEndTokenList1[i];
  }

  std::string startEndTokenList3[2];
  for(int i = 0; i < 2; i++){
    startEndTokenList3[i] = startEndTokenList1[i+2];
  }

  routes.ChooseRoute (startEndTokenList2,ueNode2); // kokono ChooseRoute no syori wo kobetu ni suru 
  routes2.ChooseRoute (startEndTokenList3,ueNode3); // kokono ChooseRoute no syori wo kobetu ni suru 

  /*
  // Wifi Install
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  Config::SetDefault( "ns3::RangePropagationLossModel::MaxRange", DoubleValue( 500.0 ) );
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiChannel.AddPropagationLoss(  "ns3::RangePropagationLossModel" );
  wifiPhy.SetChannel (wifiChannel.Create ());

  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                StringValue ("OfdmRate6Mbps"), ///
                                "RtsCtsThreshold", UintegerValue (0));
  NetDeviceContainer devices;
  devices = wifi.Install (wifiPhy, wifiMac, ueNodes);

  InternetStackHelper internet;
  internet.Install (ueNodes);

  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("192.168.0.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = ipAddrs.Assign(devices);
  
  // uint16_t port = 50000;
  // OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port)));
  onoff.SetConstantRate (DataRate ("500kb/s"));
  onoff.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.01]"));
  onoff.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.99]")); 
  
  // ApplicationContainer app;
  float sendtime = 1.0;
  for(int u = 0; u < numberOfueNodes; u++){
    if(nodeappcount[u]==1){
      app = onoff.Install (ueNodes.Get(u));
      app.Start (Seconds (sendtime));
      sendtime = sendtime + 0.1;
    }
  }

  // setup sink
  PacketSinkHelper wifiPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));

  ApplicationContainer rapp;
  rapp = wifiPacketSinkHelper.Install (ueNodes);
  rapp.Start (Seconds (1.0));

  // Uncomment to enable PCAP tracing
  // wifiPhy.EnablePcapAll("dicomomo");
  */

  // Config::Connect("NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",MakeCallback(&ReceivedPacket));
  

  // Create netanim XML to visualize the output
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("n23-zikken-03.mob"));
  

  // Animation
  // pAnim = new AnimationInterface (animFile.c_str());
  // pAnim->SetBackgroundImage ("/home/yutaka/workspace/ns-allinone-3.26/ns-3.26/scratch/map02.png", 0, 0, 2, 2, 0.8);
    AnimationInterface anim ("z23zikken-2-animation.xml"); // Mandatory

  // Omazinai
  Simulator::Run ();
  Simulator::Destroy ();
  // delete pAnim;
  return 0;
}

