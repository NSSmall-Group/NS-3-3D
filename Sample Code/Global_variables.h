#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/netanim-module.h"
#include "ns3/netsimulyzer-module.h"
#include "string"
#include "ns3/bridge-helper.h"
#include "ns3/aodv-helper.h"
#include "ns3/wifi-module.h"

namespace ns3
{
    extern YansWifiChannelHelper channel;
    extern YansWifiPhyHelper phy;
    extern WifiHelper wifi;
    extern WifiMacHelper mac;
}

void InitializeWifiSettings();

#endif // GLOBAL_VARIABLES_H

