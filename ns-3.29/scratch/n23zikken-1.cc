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
  int numberOfueNodes  = 21, numberOfueNodes_2 =  2 * numberOfueNodes;
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
  // put ueNodes
  std::vector<std::string> startEndTokenList1;
  float shel_x[21], shel_y[21];
  shel_x[0] = 39.647178;//臼木山高台
  shel_y[0] = 141.974954;
  shel_x[1] = 39.646614;//浄土ヶ浜　第1駐車場
  shel_y[1] = 141.978188;
  shel_x[2] = 39.649826;//浄土ヶ浜　第3駐車場
  shel_y[2] = 141.973416;
  shel_x[3] = 39.648375;//岩手県立水産科学館
  shel_y[3] = 141.976057;
  shel_x[4] = 39.650641;//熊野神社
  shel_y[4] = 141.968347;
  shel_x[5] = 39.649691;//鍬ケ崎小学校
  shel_y[5] = 141.966174;
  shel_x[6] = 39.646889;//梅翁寺
  shel_y[6] = 141.964478;
  shel_x[7] = 39.644741;//本照寺
  shel_y[7] = 141.963404;
  shel_x[8] = 39.643381;//常安寺分院
  shel_y[8] = 141.965628;
  shel_x[9] = 39.641840;//善林寺
  shel_y[9] = 141.964319;
  shel_x[10] = 39.641760;//大杉神社
  shel_y[10] = 141.965485;
  shel_x[11] = 39.640773;//宮古漁協ビル
  shel_y[11] = 141.966722;
  shel_x[12] = 39.643567;//中央公民館裏高台
  shel_y[12] = 141.957206;
  shel_x[13] = 39.645806;//常安寺
  shel_y[13] = 141.955117;
  shel_x[14] = 39.646897;//宮古消防署
  shel_y[14] = 141.946614;
  shel_x[15] = 39.647590;//つつじヶ丘公園高台
  shel_y[15] = 141.943700;
  shel_x[16] = 39.647000;//もみじヶ丘公園高台
  shel_y[16] = 141.933316;
  shel_x[17] = 39.642449;//館合近隣公園高台
  shel_y[17] = 141.942089;
  shel_x[18] = 39.638134;//横山八幡宮
  shel_y[18] = 141.943702;
  shel_x[19] = 39.644355;//宮古小学校
  shel_y[19] = 141.949639;
  shel_x[20] = 39.639481;//市民交流センター
  shel_y[20] = 141.946223;
      
  float star_x[21], star_y[21]; // start 
  star_x[0] = 39.638112;
  star_y[0] = 141.951008;
  star_x[1] = 39.639743;
  star_y[1] = 141.953729;
  star_x[2] = 39.642250;
  star_y[2] = 141.954802;
  star_x[3] = 39.641491; 
  star_y[3] = 141.958996; 
  star_x[4] = 39.640570; 
  star_y[4] = 141.962516;
  star_x[5] = 39.640270; 
  star_y[5] = 141.964715;
  star_x[6] = 39.640012;  
  star_y[6] = 141.966000;
  star_x[7] = 39.640987;
  star_y[7] = 141.966523;
  star_x[8] = 39.639539;
  star_y[8] = 141.969907;
  star_x[9] = 39.647532;
  star_y[9] = 141.968189;
  star_x[10] = 39.642357;
  star_y[10] = 141.944834;
  star_x[11] = 39.644784;
  star_y[11] = 141.945389;
  star_x[12] = 39.645692;
  star_y[12] = 141.942714;
  star_x[13] = 39.643849; 
  star_y[13] = 141.950862;
  star_x[14] = 39.642745;  
  star_y[14] = 141.961927;
  star_x[15] = 39.645581; 
  star_y[15] = 141.944161;
  star_x[16] = 39.640701; 
  star_y[16] = 141.950669;
  star_x[17] = 39.642023;
  star_y[17] = 141.952171;
  star_x[18] = 39.645913; 
  star_y[18] = 141.948007;
  star_x[19] = 39.641415;
  star_y[19] = 141.959798;
  star_x[20] = 39.642545; 
  star_y[20] = 141.957646;


  int shel_c = 0;
  for(int i = 0; i < numberOfueNodes; i++){
    if(i!=0 && i%5==0) shel_c++; // kokoga okasii
    char goal[100], start[100];
    snprintf(goal, 100,"%.6f, %.6f", shel_x[shel_c], shel_y[shel_c]);
    snprintf(start, 100,"%.6f, %.6f", star_x[i], star_y[i]);
    startEndTokenList1.push_back(start);
    startEndTokenList1.push_back(goal);
  }

  std::string startEndTokenList2[numberOfueNodes_2];
  for(int i = 0; i < numberOfueNodes_2; i++){
    startEndTokenList2[i] = startEndTokenList1[i];
  }

  routes.ChooseRoute (startEndTokenList2,ueNodes);

  

  /*
  // Wifi Install
  WifiHelper wifi = WifiHelper::Default ();
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  Config::SetDefault( "ns3::RangePropagationLossModel::MaxRange", DoubleValue( 500.0 ) );
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiChannel.AddPropagationLoss(  "ns3::RangePropagationLossModel" );
  wifiPhy.SetChannel (wifiChannel.Create ());
  NetDeviceContainer nodeDevices = wifi.Install (wifiPhy, wifiMac, ueNodes);

  InternetStackHelper internet;
  internet.Install (ueNodes);

  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("192.168.0.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = ipAddrs.Assign(nodeDevices);
  
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
  */

  // Uncomment to enable PCAP tracing
  // wifiPhy.EnablePcapAll("dicomomo");

  // Config::Connect("NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",MakeCallback(&ReceivedPacket));

  // Create netanim XML to visualize the output
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("n23-zikken-02.mob"));
  

  // Animation
  // pAnim = new AnimationInterface (animFile.c_str());
  // pAnim->SetBackgroundImage ("/home/yutaka/workspace/ns-allinone-3.26/ns-3.26/scratch/map02.png", 0, 0, 2, 2, 0.8);
    AnimationInterface anim ("z23zikken-1-animation.xml"); // Mandatory
  // Omazinai
  Simulator::Run ();
  Simulator::Destroy ();
  // delete pAnim;
  return 0;
}

