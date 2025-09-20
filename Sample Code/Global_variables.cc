#include "Global_variables.h"

namespace ns3
{
    // 定义全局变量
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    WifiHelper wifi;
    WifiMacHelper mac;
}
using namespace ns3;
// 初始化这些变量
void InitializeWifiSettings(void)
{
    // 初始化 `phy`，设置发射功率和信道
    phy.Set("TxPowerStart", DoubleValue(100.0));
    phy.Set("TxPowerEnd", DoubleValue(100.0));
    phy.SetChannel(channel.Create());

    // 初始化 `mac`
    Ssid ssid = Ssid("XiDian");
    mac.SetType("ns3::AdhocWifiMac", "Ssid", SsidValue(ssid));
}

