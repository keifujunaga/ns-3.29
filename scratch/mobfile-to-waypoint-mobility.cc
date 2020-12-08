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
// int nodenum;

int
main (int argc, char **argv)
{
  int numberOfueNodes = 6;
  int pre_numberOfueNodes = 3;

  // Create nodes container
  ueNodes.Create (numberOfueNodes);

  //ue.mob file open
  //waylist
  struct mob_list wpl[pre_numberOfueNodes][100] = {}; // 
  int jc[pre_numberOfueNodes] = {};
  int i = 0;

  std::ifstream ifss ("./random-waypoint-mobility.mob");
  //std::ifstream ifss ("./1-do-routes-mobility-model.mob");

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
      //std::cout << "i is " << i << std::endl;
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
      int nodenum = u % pre_numberOfueNodes;
      std::cout << "nodenum is " << nodenum << ", u is " << u <<  std::endl;
      for (int v = 0; v < jc[nodenum]; v++)
        {
          waypoint->AddWaypoint (
              Waypoint (NanoSeconds (wpl[nodenum][v].waypoint_t),
                        Vector (wpl[nodenum][v].waypoint_x, wpl[nodenum][v].waypoint_y, 1.5)));
        }
    }

  // Create netanim XML to visualize the output
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("mobfile-to-waypoint-mobility.mob"));

  // Animation
  AnimationInterface anim ("mobfile-to-waypoint-mobility.xml"); // Mandatory

  // Omazinai
  Simulator::Run ();
  Simulator::Destroy ();
  // delete pAnim;
  return 0;
}