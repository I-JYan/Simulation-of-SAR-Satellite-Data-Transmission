#include <math.h>
#include "ns3/calculatedistance.h"

namespace ns3 {

double
CalculateDistance(const Vector &a, const Vector &b)
{
    double kx = b.x - a.x;
    double ky = b.y - a.y;
    double kz = b.z - a.z;

    double euclidean_distance = sqrt (pow(kx,2) + pow(ky,2) + pow(kz,2));

    return euclidean_distance > 9512610 ? double(INFINITY) : euclidean_distance;
}

}
