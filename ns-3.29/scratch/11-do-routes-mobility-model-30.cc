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

AnimationInterface *pAnim = 0;

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
int nodeappcount[23] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1};

//Node create
NodeContainer ueNodes;
// setup onoff
uint16_t port = 50000;
OnOffHelper onoff ("ns3::UdpSocketFactory",
                   Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port)));
ApplicationContainer app;

void
ReceivedPacket (std::string context, Ptr<const Packet> packet, const Address &)
{
  std::string delim ("/");
  std::list<std::string> list_string;
  boost::split (list_string, context, boost::is_any_of (delim));
  auto itr = list_string.begin ();
  ++itr;
  ++itr;
  int kym = std::stoi (*itr); // NodeNo

  if (nodeappcount[kym] == 0)
    {
      //NS_LOG_DEBUG(context);
      NS_LOG_DEBUG ("Node " << *itr << " Received one Packet At time "
                            << Simulator::Now ().GetSeconds ());
      app = onoff.Install (ueNodes.Get (kym));
      nodeappcount[kym] = 1;
    }
}

int
main (int argc, char **argv)
{
  int numberOfueNodes = 30;
  int numberOfueNodes_2 = 2 * numberOfueNodes;
  bool verbose = false;
  double centerLat = 39.380000, centerLng = 141.955000, centerAltitude = 0;

  CommandLine cmd;
  cmd.AddValue ("verbose", "add verbose logging", verbose);
  cmd.AddValue ("centerLatitude", "set the latitude for the center of the Cartesian plane",
                centerLat);
  cmd.AddValue ("centerLongitude", "set the longitude for the center of the Cartesian plane",
                centerLng);
  cmd.AddValue ("centerAltitude", "set the longitude for the center of the Cartesian plane",
                centerAltitude);
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
  RoutesMobilityHelper routes (centerLat, centerLng, centerAltitude);
  // put ueNodes
  std::vector<std::string> startEndTokenList1;
  float shel_x[15], shel_y[15];
  shel_x[0] = 39.639195; //new_kouraku
  shel_y[0] = 141.947284;
  shel_x[1] = 39.640594; //oosaka_ousyou
  shel_y[1] = 141.947732;
  shel_x[2] = 39.641127; //kyatoru miyako
  shel_y[2] = 141.945983;
  shel_x[3] = 39.641502; //gotou hihuka
  shel_y[3] = 141.951732;
  shel_x[4] = 39.641684; //kaigosisetsu aozora
  shel_y[4] = 141.954243;
  shel_x[5] = 39.642800; //saigai koueijyutaku
  shel_y[5] = 141.954005;
  shel_x[6] = 39.643069; //miyakohoteru_sawadaya
  shel_y[6] = 141.952436;
  shel_x[7] = 39.637814; //miyako_koutougakkou
  shel_y[7] = 141.947428;
  shel_x[8] = 39.644508; //miyako_daiichibyouin
  shel_y[8] = 141.947121;
  shel_x[9] = 39.638134; //yokohama_hachimangu
  shel_y[9] = 141.943701;
  shel_x[10] = 39.642449; //tateai_kinrinkouen
  shel_y[10] = 141.942089;
  shel_x[11] = 39.644772; //haneguro_jinja
  shel_y[11] = 141.951814;
  shel_x[12] = 39.645465; //jyouanji
  shel_y[12] = 141.955520;
  shel_x[13] = 39.646898; //syoubou_honbu
  shel_y[13] = 141.946614;
  shel_x[14] = 39.647591; //tsutsujigaoka_kouen
  shel_y[14] = 141.943700;

  float star_x[30], star_y[30]; // start
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
  star_x[15] = 39.643750;
  star_y[15] = 141.948515;
  star_x[16] = 39.643874;
  star_y[16] = 141.950875;
  star_x[17] = 39.641057;
  star_y[17] = 141.946873;
  star_x[18] = 39.641214;
  star_y[18] = 141.948858;
  star_x[19] = 39.642088;
  star_y[19] = 141.948739;
  star_x[20] = 39.642182;
  star_y[20] = 141.949533;
  star_x[21] = 39.642575;
  star_y[21] = 141.953066;
  star_x[22] = 39.640032;
  star_y[22] = 141.949208;
  star_x[23] = 39.644240;
  star_y[23] = 141.945523;
  star_x[24] = 39.642444;
  star_y[24] = 141.943928;
  star_x[25] = 39.639329;
  star_y[25] = 141.945483;
  star_x[26] = 39.640461;
  star_y[26] = 141.952335;
  star_x[27] = 39.642568;
  star_y[27] = 141.953081;
  star_x[28] = 39.643877;
  star_y[28] = 141.952359;
  star_x[29] = 39.643032;
  star_y[29] = 141.951223;

  int shel_c = 0;
  for (int i = 0; i < numberOfueNodes; i++)
    {
      // if(i!=0 && i%5==0) shel_c++; // kokoga okasii
      if (i < 3 || i == 15 || i == 23)
        {
          shel_c = 8; //miyako_daiichibyouin
        }
      else if (i == 3 || i == 16)
        {
          shel_c = 11; //haneguro_jinja
        }
      else if (i == 4 || i == 17)
        {
          shel_c = 2; //kyatoru miyako
        }
      else if (i == 5 || i == 18 || i == 19 || i == 22)
        {
          shel_c = 1; //oosaka_ousyou
        }
      else if (i == 6 || i == 21 || (27 <= i && i <= 29))
        {
          shel_c = 6; //miyakohoteru_sawadaya
        }
      else if (i == 7 || i == 8)
        {
          shel_c = 4; //kaigosisetsu aozora
        }
      else if (i == 9 || i == 20 || i == 26)
        {
          shel_c = 3; //gotou hihuka
        }
      else if (i == 10 || i == 11 || i == 25)
        {
          shel_c = 0; //new_kouraku
        }
      else if (i == 12 || i == 13 || i == 14)
        {
          shel_c = 7; //miyako_koutougakkou
        }
      else if (i == 24)
        {
        shel_c = 10;  //tateai_kinrinkouen
        }
      char goal[100], start[100];
      snprintf (goal, 100, "%.6f, %.6f", shel_x[shel_c], shel_y[shel_c]);
      snprintf (start, 100, "%.6f, %.6f", star_x[i], star_y[i]);
      startEndTokenList1.push_back (start);
      startEndTokenList1.push_back (goal);
    }

  std::string startEndTokenList2[numberOfueNodes_2];
  for (int i = 0; i < numberOfueNodes_2; i++)
    {
      startEndTokenList2[i] = startEndTokenList1[i];
    }

  routes.ChooseRoute (startEndTokenList2, ueNodes); // kokono ChooseRoute no syori wo kobetu ni suru

  // Uncomment to enable PCAP tracing
  // wifiPhy.EnablePcapAll("dicomomo");

  // Config::Connect("NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",MakeCallback(&ReceivedPacket));

  // Create netanim XML to visualize the output
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("11-do-routes-mobility-model-30.mob"));

  // Animation
  // pAnim = new AnimationInterface (animFile.c_str());
  // pAnim->SetBackgroundImage ("/home/yutaka/workspace/ns-allinone-3.26/ns-3.26/scratch/map02.png", 0, 0, 2, 2, 0.8);
  AnimationInterface anim ("11-do-routes-mobility-model-30.xml"); // Mandatory
  // Omazinai
  Simulator::Run ();
  Simulator::Destroy ();
  // delete pAnim;
  return 0;
}

/*
cp 0's minimum hinanjo is miyako_daiichibyouin
cp 0's minimum distance is 56.921643
cp 1's minimum hinanjo is miyako_daiichibyouin
cp 1's minimum distance is 124.606147
cp 2's minimum hinanjo is miyako_daiichibyouin
cp 2's minimum distance is 200.969114
cp 3's minimum hinanjo is haneguro_jinja
cp 3's minimum distance is 140.856918
cp 4's minimum hinanjo is kyatoru_miyako
cp 4's minimum distance is 140.812502
cp 5's minimum hinanjo is oosaka_ousyou
cp 5's minimum distance is 240.820870
cp 6's minimum hinanjo is miyakohoteru_sawadaya
cp 6's minimum distance is 42.215205
cp 7's minimum hinanjo is gotou_hihuka
cp 7's minimum distance is 71.409523
cp 8's minimum hinanjo is gotou_hihuka
cp 8's minimum distance is 119.152114
cp 9's minimum hinanjo is kyatoru_miyako
cp 9's minimum distance is 89.916925
cp 10's minimum hinanjo is new_kouraku
cp 10's minimum distance is 152.481385
cp 11's minimum hinanjo is new_kouraku
cp 11's minimum distance is 100.901786
cp 12's minimum hinanjo is miyako_koutougakkou
cp 12's minimum distance is 183.706175
cp 13's minimum hinanjo is miyako_koutougakkou
cp 13's minimum distance is 216.131516
cp 14's minimum hinanjo is miyako_koutougakkou
cp 14's minimum distance is 277.191796
cp 15's minimum hinanjo is miyako_daiichibyouin
cp 15's minimum distance is 146.281484
cp 16's minimum hinanjo is haneguro_jinja
cp 16's minimum distance is 128.341551
cp 17's minimum hinanjo is kyatoru_miyako
cp 17's minimum distance is 76.689636
cp 18's minimum hinanjo is oosaka_ousyou
cp 18's minimum distance is 118.660381
cp 19's minimum hinanjo is oosaka_ousyou
cp 19's minimum distance is 187.379198
cp 20's minimum hinanjo is gotou_hihuka
cp 20's minimum distance is 203.132447
cp 21's minimum hinanjo is miyakohoteru_sawadaya 
cp 21's minimum distance is 77.074593
cp 22's minimum hinanjo is oosaka_ousyou
cp 22's minimum distance is 141.149425
cp 23's minimum hinanjo is miyako_daiichibyouin
cp 23's minimum distance is 140.188912
cp 24's minimum hinanjo is tateai_kinrinkouen
cp 24's minimum distance is 157.641137
cp 25's minimum hinanjo is new_kouraku
cp 25's minimum distance is 155.108857
cp 26's minimum hinanjo is gotou_hihuka
cp 26's minimum distance is 126.889487
cp 27's minimum hinanjo is miyakohoteru_sawadaya
cp 27's minimum distance is 78.532393
cp 28's minimum hinanjo is miyakohoteru_sawadaya
cp 28's minimum distance is 90.188006
cp 29's minimum hinanjo is miyakohoteru_sawadaya
cp 29's minimum distance is 104.059760
*/