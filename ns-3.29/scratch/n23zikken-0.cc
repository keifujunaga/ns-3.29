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
  int numberOfueNodes  = 23, numberOfueNodes_2 =  2 * numberOfueNodes;
  bool verbose = false;
  double centerLat = 41.771109, centerLng = 140.732804, centerAltitude = 0;

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
  float shel_x[23], shel_y[23];
  shel_x[0] = 41.778354;//函館地方裁判所
  shel_y[0] = 140.738759;
  shel_x[1] = 41.777953;//函館パークホテル
  shel_y[1] = 140.737548;
  shel_x[2] = 41.777198;//総合福祉センター
  shel_y[2] = 140.730697;
  shel_x[3] = 41.774070;//ホテルグランディア函館駅前
  shel_y[3] = 140.728171;
  shel_x[4] = 41.774058;//函館中央郵便局
  shel_y[4] = 140.734094;
  shel_x[5] = 41.772679;//ロワジール
  shel_y[5] = 140.726510;
  shel_x[6] = 41.771460;//東横イン函館駅前朝市
  shel_y[6] = 140.724666;
  shel_x[7] = 41.772125;//コンフォートホテル函館
  shel_y[7] = 140.727956;
  shel_x[8] = 41.771074;//東横イン函館大門
  shel_y[8] = 140.730310;
  shel_x[9] = 41.770004;//ホテルパコ函館
  shel_y[9] = 140.735599;
  shel_x[10] = 41.768905;//函館市消防本部
  shel_y[10] = 140.727220;
  shel_x[11] = 41.767713;//ラビスタ函館ベイ
  shel_y[11] = 140.718970;
  shel_x[12] = 41.767505;//道営旭森団地
  shel_y[12] = 140.726961;
  shel_x[13] = 41.766246;//ホテルショコラ
  shel_y[13] = 140.724090;
  shel_x[14] = 41.766640;//あさひ小学校
  shel_y[14] = 140.732425;
  shel_x[15] = 41.765554;//メゾンプレシデント
  shel_y[15] = 140.722089;
  shel_x[16] = 41.764990;//豊川コモンズ
  shel_y[16] = 140.719905;
  shel_x[17] = 41.763847;//特定公共賃貸住宅豊川団地
  shel_y[17] = 140.720740;
  shel_x[18] = 41.763514;//豊川改良住宅
  shel_y[18] = 140.720692;
  shel_x[19] = 41.763286;//道営住宅高田屋通団地
  shel_y[19] = 140.722052;
  shel_x[20] = 41.761932;//道営住宅であえ～る大森浜団地Ｂ
  shel_y[20] = 140.726585;
  shel_x[21] = 41.761784;//道営住宅であえ～る大森浜団地A
  shel_y[21] = 140.725995;
  shel_x[22] = 41.763660;//アクロス十字街
  shel_y[22] = 140.717853;
  shel_x[23] = 41.763044;//ヴィラコンコルディア
  shel_y[23] = 140.717075;
      
  float star_x[23], star_y[23]; // start 
  star_x[0] = 41.782673;
  star_y[0] = 140.735214;
  star_x[1] = 41.780495;
  star_y[1] = 140.732727;
  star_x[2] = 41.778812;
  star_y[2] = 140.726135;
  star_x[3] = 41.773621;
  star_y[3] = 140.723434;
  star_x[4] = 41.774509;
  star_y[4] = 140.731247;
  star_x[5] = 41.774384;
  star_y[5] = 140.731506;
  star_x[6] = 41.767197;
  star_y[6] = 140.726868;
  star_x[7] = 41.774910;
  star_y[7] = 140.727219;
  star_x[8] = 41.770893;
  star_y[8] = 140.734558;
  star_x[9] = 41.768555;
  star_y[9] = 140.739700;
  star_x[10] = 41.768787;
  star_y[10] = 140.728836;
  star_x[11] = 41.769173;
  star_y[11] = 140.719727;
  star_x[12] = 41.766308;
  star_y[12] = 140.722626;
  star_x[13] = 41.768589;
  star_y[13] = 140.719513;
  star_x[14] = 41.766693;
  star_y[14] = 140.733917;
  star_x[15] = 41.764713;
  star_y[15] = 140.718521;
  star_x[16] = 41.767822;
  star_y[16] = 140.716476;
  star_x[17] = 41.761478;
  star_y[17] = 140.717239;
  star_x[18] = 41.767715;
  star_y[18] = 140.719345;
  star_x[19] = 41.761051;
  star_y[19] = 140.722214;
  star_x[20] = 41.763004;
  star_y[20] = 140.728027;
  star_x[21] = 41.758701;
  star_y[21] = 140.727615;
  star_x[22] = 41.762138;
  star_y[22] = 140.719589;

  int shel_c = 0;
  for(int i = 0; i < numberOfueNodes; i++){
    if(i!=0 && i%5==0) shel_c++;
    char goal[24], start[24];
    sprintf(goal, "%.6f, %.6f", shel_x[shel_c], shel_y[shel_c]);
    sprintf(start, "%.6f, %.6f", star_x[i], star_y[i]);
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
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("zikkenwifi.mob"));
  

  // Animation
   pAnim = new AnimationInterface (animFile.c_str());
  // pAnim->SetBackgroundImage ("/home/yutaka/workspace/ns-allinone-3.26/ns-3.26/scratch/map02.png", 0, 0, 2, 2, 0.8);
  // AnimationInterface anim ("z23zikken-0-animation.xml"); // Mandatory
  // Omazinai
  Simulator::Run ();
  Simulator::Destroy ();
  // delete pAnim;
  return 0;
}

