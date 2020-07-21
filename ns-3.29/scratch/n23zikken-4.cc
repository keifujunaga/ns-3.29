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

struct mob_list
{ // zahyou no hensu
  int node_no;
  double waypoint_t;
  double waypoint_x;
  double waypoint_y;
  double waypoint_z;
};

//Node create
NodeContainer ueNodes;

int
main (int argc, char **argv)
{
  int numberOfueNodes = 15;
  bool verbose = false;
  double centerLat = 39.380000, centerLng = 141.955000, centerAltitude = 0;

  std::string animFile = "dicomo-N23-ETWS+ProSe.xml";

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

  //ue.mob file open
  //waylist
  struct mob_list wpl[numberOfueNodes][100] = {}; // 
  int jc[numberOfueNodes] = {};
  int i = 0;

  std::ifstream ifss ("./n23-zikken-02.mob");
  std::string str;
  if (ifss.fail ())
    {
      std::cerr << "失敗" << std::endl;
      return -1;
    }

  while (std::getline (ifss, str))
    {
      std::string delim ("= :");
      std::list<std::string> list_waypointstr;
      boost::split (list_waypointstr, str, boost::is_any_of (delim));
      auto itr2 = list_waypointstr.begin ();
      ++itr2; //waypointtime
      ++itr2; //node
      ++itr2; //nodeno
      i = std::stoi (*itr2);
      wpl[i][jc[i]].node_no = std::stoi (*itr2);
      --itr2; //node
      --itr2; //waypointtime
      std::string ytk = *itr2;
      ytk.erase (--ytk.end ());
      ytk.erase (--ytk.end ());
      wpl[i][jc[i]].waypoint_t = std::stod (ytk);
      ++itr2; //node
      ++itr2; //nodeno
      ++itr2; //pos
      ++itr2; //x
      wpl[i][jc[i]].waypoint_x = std::stod (*itr2);
      ++itr2; //y
      wpl[i][jc[i]].waypoint_y = std::stod (*itr2);
      ++itr2; //z
      wpl[i][jc[i]].waypoint_z = std::stod (*itr2);
      jc[i]++;
    }

  MobilityHelper mobility;
  //UE set AddWaypoint ga mobility wo seisei ?
  mobility.SetMobilityModel ("ns3::WaypointMobilityModel");
  mobility.Install (ueNodes);
  for (int u = 0; u < numberOfueNodes; u++)
    {
      Ptr<WaypointMobilityModel> waypoint =
          DynamicCast<WaypointMobilityModel> (ueNodes.Get (u)->GetObject<MobilityModel> ());
      for (int v = 0; v < jc[u]; v++)
        {
          /*
          waypoint->AddWaypoint (
              Waypoint (NanoSeconds (wpl[u][v].waypoint_t),
                        Vector (wpl[u][v].waypoint_x, wpl[u][v].waypoint_y, wpl[u][v].waypoint_z)));
          */
         waypoint->AddWaypoint (
              Waypoint (NanoSeconds (wpl[u][v].waypoint_t),
                        Vector (wpl[u][v].waypoint_x+500, wpl[u][v].waypoint_y-29000, 1.5)));
        }
    }

  // Create netanim XML to visualize the output
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("n23-zikken-04.mob"));

  // Animation
  AnimationInterface anim ("z23zikken-4-animation.xml"); // Mandatory

  // Omazinai
  Simulator::Run ();
  Simulator::Destroy ();
  // delete pAnim;
  return 0;
}