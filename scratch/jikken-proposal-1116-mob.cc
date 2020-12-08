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
NodeContainer olsrContainer;
NodeContainer dtnContainer;
// int nodenum;

int
main (int argc, char **argv)
{
  double appDataStart = 0.0;
  //double appDataEnd = 60000;
  double appDataEnd = 1000;
  double sim_total_time = appDataStart + appDataEnd; // simulator stop time
  int total_node_num = 9;
  int uav_num = 6;
  int vehicle_num = 3;

  struct mob_list wpl[vehicle_num][1000] = {}; //
  int jc[vehicle_num] = {};
  int i = 0;

  // 順番に充電場所を変える
  // double uav_battery_time[] = {2.5, 2.5 * 2, 2.5 * 3, 2.5 * 4, 2.5 * 5, 2.5 * 6};
  // 一斉に充電場所を変える
 // double uav_battery_time[] = {960, 960, 960, 960, 960, 960};
  double uav_battery_time [] = {16, 16, 16, 16, 16, 16};
  //int hovering_time = 960;
  int hovering_time = 16;
  //int one_way_time = 360;
  int one_way_time = 6;
  //int battery_change_time = 240;
  int battery_change_time = 4;
  std::map<int, std::map<int, double>> waypoint_time;
  std::map<int, double> add_waypoint;
  int j_count = 0;
  double waypoint_count = 0;

  // モバイル端末用のWaypointMobilityを抽出
  std::ifstream ifss ("./random-waypoint-mobility-2-2.mob");
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


  for (int i = 0; i < uav_num; i++)
    {
      j_count = 0; // reset
      for (double j = uav_battery_time[i];
           j + one_way_time * 2 + battery_change_time < sim_total_time; j = j + hovering_time)
        {
          std::cout << "j_count is " << j_count << std::endl;

          j_count++;
          waypoint_time[i][j_count] = j; // 15分, 15+6+1+6+15分

          j_count++;
          waypoint_time[i][j_count] = j + one_way_time; // 15+6分, 15+6+1+6+15+6分
          j += one_way_time;

          j_count++;
          waypoint_time[i][j_count] = j + battery_change_time; // 15+6+1分, 15+6+1+6+15+6+1分
          j += battery_change_time;

          j_count++;
          waypoint_time[i][j_count] = j + one_way_time; // 15+6+1+6分, 15+6+1+6+15+6+1+6分
          j += one_way_time;
        }
    }

  olsrContainer.Create (total_node_num);
  dtnContainer.Create (total_node_num);

  /*
   *      Mobility model Setup
   */
  MobilityHelper mobility;
  MobilityHelper mobility2;

  Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> initialAlloc2 = CreateObject<ListPositionAllocator> ();

  initialAlloc->Add (Vector (500, 500, 200));
  initialAlloc->Add (Vector (1500, 500, 200));
  initialAlloc->Add (Vector (2500, 500, 200));
  initialAlloc->Add (Vector (500, 1500, 200));
  initialAlloc->Add (Vector (1500, 1500, 200));
  initialAlloc->Add (Vector (2500, 1500, 200));
  initialAlloc2->Add (Vector (200, 1300, 1.5));
  //initialAlloc2->Add (Vector (1500, 1500, 1.5));
  initialAlloc2->Add (Vector (1500, 500, 1.5));
  initialAlloc2->Add (Vector (2500, 1500, 1.5));

  mobility.SetPositionAllocator (initialAlloc);
  mobility2.SetPositionAllocator (initialAlloc2);

  mobility.SetMobilityModel ("ns3::WaypointMobilityModel");
  //mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  for (int i = 0; i < uav_num; i++)
    {
      mobility.Install (olsrContainer.Get (i));
      mobility.Install (dtnContainer.Get (i));
    }

  for (int i = 0; i < uav_num; i++)
    {
      Ptr<WaypointMobilityModel> uav_waypoint =
          DynamicCast<WaypointMobilityModel> (olsrContainer.Get (i)->GetObject<MobilityModel> ());

      Ptr<WaypointMobilityModel> uav_waypoint2 =
          DynamicCast<WaypointMobilityModel> (dtnContainer.Get (i)->GetObject<MobilityModel> ());

      waypoint_count = -1;

      for (uint32_t j = 1; j < waypoint_time[i].size (); j++)
        {
          std::cout << "1. waypointtime[" << i << "][" << j << "] is " << waypoint_time[i][j]
                    << std::endl;
          if (waypoint_count < waypoint_time[i][j])
            {
              // 2.66分
              uav_waypoint->AddWaypoint (
                  Waypoint (Seconds (waypoint_time[i][j]),
                            Vector (i % 3 * 1000 + 500, i / 3 * 1000 + 500, 200)));

              uav_waypoint2->AddWaypoint (
                  Waypoint (Seconds (waypoint_time[i][j]),
                            Vector (i % 3 * 1000 + 500, i / 3 * 1000 + 500, 200)));
            }
          waypoint_count = waypoint_time[i][j];
          j++;

          std::cout << "2. waypointtime[" << i << "][" << j << "] is " << waypoint_time[i][j]
                    << std::endl;
          if (waypoint_count < waypoint_time[i][j])
            {
              // 2.66+6分
              uav_waypoint->AddWaypoint (
                  Waypoint (Seconds (waypoint_time[i][j]),
                            Vector (i % 3 * 1000 + 500, i / 3 * 1000 + 500 + 5000, 200)));

              uav_waypoint2->AddWaypoint (
                  Waypoint (Seconds (waypoint_time[i][j]),
                            Vector (i % 3 * 1000 + 500, i / 3 * 1000 + 500 + 5000, 200)));
            }
          waypoint_count = waypoint_time[i][j];
          j++;

          std::cout << "3. waypointtime[" << i << "][" << j << "] is " << waypoint_time[i][j]
                    << std::endl;
          if (waypoint_count < waypoint_time[i][j])
            {
              // 2.66+6分
              uav_waypoint->AddWaypoint (
                  Waypoint (Seconds (waypoint_time[i][j]),
                            Vector (i % 3 * 1000 + 500, i / 3 * 1000 + 500 + 5000, 200)));

              uav_waypoint2->AddWaypoint (
                  Waypoint (Seconds (waypoint_time[i][j]),
                            Vector (i % 3 * 1000 + 500, i / 3 * 1000 + 500 + 5000, 200)));
            }
          waypoint_count = waypoint_time[i][j];
          j++;

          std::cout << "4. waypointtime[" << i << "][" << j << "] is " << waypoint_time[i][j]
                    << std::endl;
          if (waypoint_count < waypoint_time[i][j])
            {
              // 2.66+6+6分
              uav_waypoint->AddWaypoint (
                  Waypoint (Seconds (waypoint_time[i][j]),
                            Vector (i % 3 * 1000 + 500, i / 3 * 1000 + 500, 200)));

              uav_waypoint2->AddWaypoint (
                  Waypoint (Seconds (waypoint_time[i][j]),
                            Vector (i % 3 * 1000 + 500, i / 3 * 1000 + 500, 200)));
            }
          waypoint_count = waypoint_time[i][j];
        }
    }

  mobility2.SetMobilityModel ("ns3::WaypointMobilityModel");

  for (int i = uav_num; i < total_node_num; i++)
    {
      mobility2.Install (olsrContainer.Get (i));
      mobility2.Install (dtnContainer.Get (i));
    }

  for (int u = uav_num; u < total_node_num; u++)
    {
      Ptr<WaypointMobilityModel> waypoint =
          DynamicCast<WaypointMobilityModel> (olsrContainer.Get (u)->GetObject<MobilityModel> ());

      Ptr<WaypointMobilityModel> waypoint2 =
          DynamicCast<WaypointMobilityModel> (dtnContainer.Get (u)->GetObject<MobilityModel> ());

      //NS_LOG_INFO ("u-vehicle_num is " << u - uav_num);

      for (int v = 0; v < jc[u - uav_num]; v++)
        {
          std::cout << "wpl[" << u - uav_num << "][" << v << "] is "
                    << wpl[u - uav_num][v].waypoint_t << std::endl;
          waypoint->AddWaypoint (Waypoint (
              NanoSeconds (wpl[u - uav_num][v].waypoint_t),
              Vector (wpl[u - uav_num][v].waypoint_x, wpl[u - uav_num][v].waypoint_y, 1.5)));
          waypoint2->AddWaypoint (Waypoint (
              NanoSeconds (wpl[u - uav_num][v].waypoint_t),
              Vector (wpl[u - uav_num][v].waypoint_x, wpl[u - uav_num][v].waypoint_y, 1.5)));
        }
    }

  Simulator::Stop (Seconds (sim_total_time));

  // Create netanim XML to visualize the output
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("jikken-proposal-1116-mob.mob"));

  // Animation
  AnimationInterface anim ("jikken-proposal-1116-mob.xml"); // Mandatory

  for (int i = 0; i < uav_num; i++)
    {
      anim.UpdateNodeColor (i, 255, 0, 0);
      anim.UpdateNodeSize (i, 100, 100);
    }

  for (int i = uav_num; i < uav_num + vehicle_num; i++)
    {
      anim.UpdateNodeColor (i, 0, 0, 255);
      anim.UpdateNodeSize (i, 100, 100);
    }

  for (int i = uav_num + vehicle_num; i < uav_num + vehicle_num + uav_num; i++)
    {
      anim.UpdateNodeColor (i, 255, 0, 0);
      anim.UpdateNodeSize (i, 100, 100);
    }

  for (int i = uav_num + vehicle_num + uav_num; i < uav_num + vehicle_num + uav_num + vehicle_num;
       i++)
    {
      anim.UpdateNodeColor (i, 0, 0, 255);
      anim.UpdateNodeSize (i, 100, 100);
    }

  // Omazinai
  Simulator::Run ();
  Simulator::Destroy ();
  // delete pAnim;
  return 0;
}