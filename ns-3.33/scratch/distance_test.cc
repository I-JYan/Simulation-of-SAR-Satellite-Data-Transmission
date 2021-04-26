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
 */ 


#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include <math.h>

using namespace ns3;

double
distance(const Vector3D &a, const Vector3D &b)
{
    double kx = b.x - a.x;
    double ky = b.y - a.y;
    double kz = b.z - a.z;

    double euclidean_distance = sqrt (pow(kx,2) + pow(ky,2) + pow(kz,2));

    return euclidean_distance > 9512610 ? double(INFINITY) : euclidean_distance;
}

int main (int argc, char *argv[])
{
  Vector v2(0,0,7064000);
  Vector v1(0,0, -6371000);
  // Vector v1(6371000, 0, 0);
  double d = distance(v1, v2);
  std::cout << "distance =" << d <<std::endl;

  return 0;
}

