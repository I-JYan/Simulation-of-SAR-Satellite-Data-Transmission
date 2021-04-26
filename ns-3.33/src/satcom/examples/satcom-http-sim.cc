/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
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
 */ 
/*
 * ENSC894 - Communication Networks
 * Team O2
 * Author: Jie Yan
 * Note: Some code snippets are cited from the offical ns-3 tutorials and API documentations,
 * and the example usage of some functions comes from Github open source code base,
 * more specifically,
 * ns-allinone-3.33/ns-3.33/src/applications/examples/three-gpp-http-example.cc
 * ns-allinone-3.33/ns-3.33/src/applications/test/bulk-send-application-test-suite.cc
 * ns-allinone-3.33/ns-3.33/examples/tutorial/third.cc
 * are partly cited and referenced when writing this code.
 * and
 * https://github.com/anysam/starlink_ns3/blob/master/ns-allinone-3.30.1/ns-3.30.1/src/leo-satellite/model/leo-satellite-config.cc
 * is referenced for example usage of some ns-3 functions.
 * This code is based on the Network Simulator 3 [1],
 * [1] Carneiro, Gustavo. "NS-3: Network simulator 3." UTM Lab Meeting April. Vol. 20. 2010.
 */
 
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/sar-orbit-mobility-model.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/calculatedistance.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

// namespace uses
using namespace ns3;
using namespace std;

// Default Network Topology (for only two ground stations)
//
// Ground Station                     Ground Station
//                     Satellite
//              10.1.2.0         10.1.1.0
//       n2 -------------- n0 -------------- n1
//           point-to-point    point-to-point

// define some constants
#define SPEED_OF_LIGHT 299792458

// declare some functions
void CourseChange (string context, Ptr<const MobilityModel> model);

NS_LOG_COMPONENT_DEFINE ("satcom-http-sim");

# define some global variables
NodeContainer ground_station_nodes;
NodeContainer satellite_node;
uint32_t ground_station_num = 2;
// define a vector container to hold returned NetDevices
vector<NetDeviceContainer> net_devices_vec;
// create a vector container to hold returned Ipv4Interfaces
vector<Ipv4InterfaceContainer> ip_interfaces_vec;

int 
main (int argc, char *argv[])
{
  // default values for some passed arguments
  // the switch for verbosity of the application modules
  bool verbose = true;
  // the switch for PCAP tracing
  bool tracing = true;
  // the default file name for flow monitor output
  string flow_monitor_file_name = "capture-http.xml";
  // the string stream as a temporary string holder when printing information
  stringstream info_stream;
  
  // add commands and parse the command line
  CommandLine cmd (__FILE__);
  cmd.AddValue ("nGroundStation", "Number of Ground Stations", ground_station_num);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue ("xml_file", "The XML file name for Flow Monitor", flow_monitor_file_name);
  cmd.Parse (argc, argv);

  // there should be at least two ground stations
  if (ground_station_num < 2){
      NS_LOG_ERROR ("nWifi should be greater than 2");
      return 1;
    }
  // turn on component logging if verbosity is true
  if (verbose){
      LogComponentEnable ("ThreeGppHttpClient", LOG_LEVEL_INFO);
      LogComponentEnable ("ThreeGppHttpServer", LOG_LEVEL_INFO);
    }
  
  // create ground station nodes
  ground_station_nodes.Create (ground_station_num);
    
  // create satellite node
  satellite_node.Create (1); // there is only one satellite for SAR
  
  // set mobility model for the ground stations
  MobilityHelper ground_station_mobility;
  // create a position allocator to initialize the initial positions
  // for ground stations
  Ptr<ListPositionAllocator> posAllocator = CreateObject<ListPositionAllocator>();
  // initial positions for the ground station nodes
  // North Pole
  posAllocator->Add(Vector(0.0,0.0,6371000.0));
  // South Pole
  posAllocator->Add(Vector(0.0,0.0,-6371000.0)); 
  // tell the helper function to use the position allocator
  ground_station_mobility.SetPositionAllocator(posAllocator);
  // ground stations' position remains fixed
  ground_station_mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  // install the mobility model on the ground station nodes
  ground_station_mobility.Install(ground_station_nodes);
  
  // print initial positions of ground stations
  for (unsigned i = 0; i < ground_station_num; i++){
      // retrieve the mobility model
      Ptr<MobilityModel> mobilityModel = ground_station_nodes.Get(i)->GetObject<MobilityModel> ();
      // retrieve the current position
      Vector pos = mobilityModel->GetPosition();
      info_stream << "The " << i << "th ground station is initially at: " << "x = " << pos.x
                  << ", y = " << pos.y << ", z = " << pos.z << endl;
      // print the info
      NS_LOG_DEBUG(info_stream.str());
      // clear the buffer
      info_stream.str("");
  }
  
  
  // set mobility model for the satellite
  MobilityHelper satellite_mobility;
  // set to the SAR mobility model 
  satellite_mobility.SetMobilityModel ("ns3::SarOrbitMobilityModel", "Timestep", StringValue("1481.1425s"));
  // install the mobility model on the satellite node
  satellite_mobility.Install(satellite_node);
  
  // receive the course change notification
  // construct the trace path
  info_stream << "/NodeList/" << satellite_node.Get(0)->GetId () << "/$ns3::MobilityModel/CourseChange";
  // for debug
  NS_LOG_DEBUG(info_stream.str());
  // connect the callback
  Config::Connect (info_stream.str(), MakeCallback (&CourseChange));
  //clear the stream
  info_stream.str("");
  
  // connect each ground station to the single satellite
  // define a helper for point-to-point link
  PointToPointHelper pointToPoint;
  
  for (unsigned i = 0; i < ground_station_num; i++){
      // retrieve the location of the ground station
      Vector ground_station_location = ground_station_nodes.Get(i)->GetObject<MobilityModel>()->GetPosition();
      // retrieve the location of the satellite
      Vector satellite_location = satellite_node.Get(0)->GetObject<MobilityModel>()->GetPosition();
      // compute the distance between the two communicating entities
      double station_satellite_distance = CalculateDistance(ground_station_location, satellite_location);
      // compute the propagation delay
      double popagation_delay = (station_satellite_distance * 1000) / SPEED_OF_LIGHT;
      // set P2P link properties
      pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("520Mbps"));
      pointToPoint.SetChannelAttribute ("Delay", TimeValue(Seconds (popagation_delay)));
      // install the channel between the ground station and the satellite
      NetDeviceContainer p2pDevices = pointToPoint.Install(ground_station_nodes.Get(i), satellite_node.Get(0));
      // add these net devices to the vector
      net_devices_vec.push_back(p2pDevices);
      // print info
      info_stream << "Registered ground station #" << i << " distance: " << station_satellite_distance
                  << "km, delay: " << popagation_delay << "s" << endl;
      NS_LOG_INFO(info_stream.str());
      //clear the stream
      info_stream.str("");
      
  }
  
  // install internet protocol stack
  InternetStackHelper stack;
  stack.Install(ground_station_nodes);
  stack.Install(satellite_node);
  
  // assign IPv4 addresses to these nodes
  
  // create a IPv4 address helper
  Ipv4AddressHelper address;
  // set base addresses
  address.SetBase ("10.1.0.0", "255.255.255.0");
  
  for(unsigned i = 0; i < ground_station_num; i++){
      // switch to a new subnet before address assignment
      address.NewNetwork();
      // assign the IPv4 addresses to this subnet
      Ipv4InterfaceContainer p2pInterfaces = address.Assign(net_devices_vec[i]);
      // add these interfaces to the vector
      ip_interfaces_vec.push_back(p2pInterfaces);
  }
  // install the HTTP server on the last ground station
  ThreeGppHttpServerHelper httpServer (ip_interfaces_vec[ground_station_num - 1].GetAddress(0));
  ApplicationContainer serverApps = httpServer.Install (ground_station_nodes.Get (ground_station_num - 1));
  // get the HTTP server application
  // in order to configure its parameters 
  Ptr<ThreeGppHttpServer> http_server_app = serverApps.Get (0)->GetObject<ThreeGppHttpServer> ();
  // configure the HTTP server
  // citing three-gpp-http-example.cc
  PointerValue varPtr;
  http_server_app->GetAttribute ("Variables", varPtr);
  Ptr<ThreeGppHttpVariables> httpVariables = varPtr.Get<ThreeGppHttpVariables>();
  // the mean size of HTTP objects is 100KB 
  httpVariables->SetMainObjectSizeMean (102400);
  // the standard deviation of HTTP objects is 40KB
  httpVariables->SetMainObjectSizeStdDev (40960);
  // set the start time and the stop time of the server
  // the server starts at 1s and stops at 12000s
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (12000.0));
  
  // install clients on the remaining ground stations
  for (unsigned i = 0; i < ground_station_num - 1; i++){
    // create a HTTP client communicating with the last ground station at port 80
	// 80 is the default port # for HTTP
    ThreeGppHttpClientHelper httpClient (ip_interfaces_vec[ground_station_num - 1].GetAddress(0));
	// install the HTTP client on all ground stations but the last one
    ApplicationContainer clientApps = httpClient.Install (ground_station_nodes.Get(i));
    // set the start time and the stop time of the client
	// the client starts at 2s
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (12000.0));
  }
  
  // automatically populate the routing tables for all nodes
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // set the time when the simualtion should stop
  Simulator::Stop (Seconds (12000.0));
  
  // turn on PCAP tracing on all nodes if tracing is switched on (on by default)
  if (tracing == true)
    {
      // save the tracing files prefixed with "satcom-http-sim"
      pointToPoint.EnablePcapAll ("satcom-http-sim");
    }
    
  // enable flow monitor on all nodes
  FlowMonitorHelper flowmonHelper;
  flowmonHelper.InstallAll();
  // enable NetAnim tracing on all nodes
  // this provides some sort of animation
  AnimationInterface anim ("animation-http.xml");
  anim.EnableIpv4RouteTracking ("route-http.xml", Seconds (0.0), Seconds (12000.0), Seconds(0.5));
  anim.SetMobilityPollInterval (Seconds (0.5));
  anim.EnablePacketMetadata (true);
  
  // run the simulation
  Simulator::Run ();
  // save the flow monitor tracing file to disk
  flowmonHelper.SerializeToXmlFile(flow_monitor_file_name, true, true);
  // release the resources associated with this simulation
  Simulator::Destroy ();
  // termination of this function
  return 0;
}

void
CourseChange (std::string context, Ptr<const MobilityModel> model)
{
    // create an info string stream
    stringstream info_stream;
    // get the new coordinates
    Vector satellite_location = model->GetPosition();
    // construct info message
    info_stream << context << " new location for the satellite: x = " << satellite_location.x
         << ", y = " << satellite_location.y << ", z = " << satellite_location.z << endl;
    // print info
    NS_LOG_INFO(info_stream.str());
    //clear the stream
    info_stream.str("");
    // update the links
    for (unsigned i = 0; i < ground_station_num; i++){
        // retrieve the location of the ground station
        Vector ground_station_location = ground_station_nodes.Get(i)->GetObject<MobilityModel>()->GetPosition();
        // get the channel of this ground station
        Ptr<Channel> channel = net_devices_vec[i].Get(0)->GetChannel();
        // get the specific P2P Channel associated with this station
        Ptr<PointToPointChannel> p2p_channel = channel->GetObject<PointToPointChannel>();
        // compute the new distance
        // compute the distance between the two communicating entities
        double station_satellite_distance = CalculateDistance(ground_station_location, satellite_location);
        // compute the propagation delay
        double popagation_delay = (station_satellite_distance * 1000) / SPEED_OF_LIGHT;
        // update the link delay
        p2p_channel->SetAttribute("Delay", TimeValue(Seconds (popagation_delay)));
        // print info
        info_stream << "Registered ground station #" << i << " distance: " << station_satellite_distance
                  << "km, delay: " << popagation_delay << "s" << endl;
        NS_LOG_INFO(info_stream.str());
        //clear the stream
        info_stream.str("");
    }
    
}
