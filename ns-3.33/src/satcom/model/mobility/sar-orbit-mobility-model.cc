/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Gustavo Carneiro  <gjc@inescporto.pt>
 */
#include "sar-orbit-mobility-model.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SarOrbitMobilityModel);

TypeId SarOrbitMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SarOrbitMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<SarOrbitMobilityModel> ()
    .AddAttribute ("Timestep",
                "Update position and velocity after moving for this delay.",
                TimeValue (Seconds (1.0)),
                MakeTimeAccessor (&SarOrbitMobilityModel::m_timeStep),
                MakeTimeChecker ());
  return tid;
}

SarOrbitMobilityModel::SarOrbitMobilityModel ()
{
}

SarOrbitMobilityModel::~SarOrbitMobilityModel ()
{
}

void
SarOrbitMobilityModel::DoInitialize (void)
{
  DoInitializePrivate ();
  MobilityModel::DoInitialize ();
}

void
SarOrbitMobilityModel::DoInitializePrivate (void)
{
  DoUpdate ();
}

inline Vector
SarOrbitMobilityModel::DoGetPosition (void) const
{   
    return m_position;
}

void 
SarOrbitMobilityModel::DoSetPosition (const Vector &position)
{
  if (!m_started)
  {
    m_baseTime = Simulator::Now ();
    m_started = true;
  }

  m_position = position;

  m_event.Cancel ();
  m_event = Simulator::ScheduleNow (&SarOrbitMobilityModel::DoUpdate, this);
}

Vector
SarOrbitMobilityModel::DoGetVelocity (void) const
{
  return m_velocity;
}

void
SarOrbitMobilityModel::DoUpdate (void)
{
  if (!m_started)
  {
    m_baseTime = Simulator::Now ();
    m_started = true;
  }

  double t = (Simulator::Now () - m_baseTime).GetSeconds ();
  double angle_inc = fmod(t,time_per_orbit) / time_per_orbit * 2 * 3.14159265358979323846;
  double angle_az = floor(t/time_per_orbit) * 2 * 3.14159265358979323846 / 175;

  double x = orbit_radius * cos (angle_az) * sin (angle_inc);
  double y = orbit_radius * sin (angle_az) * sin (angle_inc);
  double z = orbit_radius * cos (angle_inc);
  m_position = Vector (x,y,z);

  double coef =  2 * 3.14159265358979323846 / time_per_orbit;
  double vx = orbit_radius * cos (angle_az) * cos (angle_inc) *  coef ;
  double vy = orbit_radius * sin (angle_az) * cos (angle_inc) * coef;
  double vz = orbit_radius * -1 * sin (angle_inc) * coef;
  m_velocity = Vector (vx, vy, vz);

  m_event.Cancel ();
  m_event = Simulator::Schedule (m_timeStep, &SarOrbitMobilityModel::DoUpdate, this);

  NotifyCourseChange ();
}

void
SarOrbitMobilityModel::DoDispose (void)
{
  // chain up
  MobilityModel::DoDispose ();
}

} // namespace ns3
