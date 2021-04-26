/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2021 Bingqian Zhang
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
 * Author: Bingqian Zhang <berilz@sfu.ca>
 */ 

/**
 * Tests sar-orbit-mobility-model extension to mobility module
 *
 * Simulates a SAR satellite orbiting around the earth twice at different azimuth angles,
 * passing the North Pole and South Pole every orbit.
 */

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

static void 
CourseChange (std::string foo, Ptr<const MobilityModel> mobility)
{
  /* Prints current position and velocity */
  Vector pos = mobility->GetPosition ();
  Vector vel = mobility->GetVelocity ();
  std::cout << Simulator::Now () << ", POS: x=" << pos.x << ", y=" << pos.y
            << ", z=" << pos.z << "; VEL: x=" << vel.x << ", y=" << vel.y
            << ", z=" << vel.z << std::endl;
}

int main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  NodeContainer c;
  c.Create (1);

  /* Set initial position at the North Pole */
  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator", 
                                          "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=0]"),
                                          "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=0]"),
                                          "Z", StringValue("ns3::UniformRandomVariable[Min=7064000|Max=7064000]"));

  /* Set time step to display position and velocity every quarter of an orbit around the earth */
  mobility.SetMobilityModel ("ns3::SarOrbitMobilityModel", "Timestep", StringValue("1481.1425s"));
  mobility.InstallAll();

  Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
                   MakeCallback (&CourseChange));

  /* Each orbit takes around 6000 seconds */
  Simulator::Stop (Seconds (12000));

  Simulator::Run ();

  Simulator::Destroy ();
  return 0;
}
