#ifndef SAR_ORBIT_MOBILITY_H
#define SAR_ORBIT_MOBILITY_H

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/mobility-model.h"
#include "ns3/vector.h"
#include "ns3/nstime.h"

namespace ns3 {

class SarOrbitMobilityModel : public MobilityModel
{
public:
    static TypeId GetTypeId (void);
    SarOrbitMobilityModel();
    virtual ~SarOrbitMobilityModel();

private:
    virtual Vector DoGetPosition (void) const;
    virtual void DoSetPosition (const Vector &position);
    virtual Vector DoGetVelocity (void) const;
    virtual void DoUpdate(void);

    void DoInitializePrivate (void);
    virtual void DoDispose (void);
    virtual void DoInitialize (void);

    bool m_started = false;

    Time m_baseTime;  //!< the base time
    Time m_timeStep;

    Vector m_position;
    Vector m_velocity;
    
    double time_per_orbit = 5924.57;
    double orbit_radius = 7064000;
    EventId m_event;
};

} // namespace ns3

#endif /* SAR_ORBIT_MOBILITY_H */