#ifndef BATTALION_H
#define BATTALION_H

#include "company.h"
#include "platoon.h"

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

using namespace ns3;

/*
control_subnet:指挥控制子网
support_subnet:后勤支持子网
collection_subnet:情报收集子网
communication_subnet:通信子网
*/
//营级编制，指挥控制子网+后勤支持子网+情报收集监控子网+通信子网p2p连接，营级通信子网通过路由器和连级通信子网交互
//10个teminal节点+1个交换机+1个路由器
typedef struct battalion
{
	NodeContainer terminals;
	NodeContainer switches;
	NodeContainer router;
	//设置网络设备用于分配ip
	NetDeviceContainer terminalDevices,switchDevices,p2pDevices,apDevices,terminalp2pDevices;
	//设置子网容器，能够获取他们的ip
	Ipv4InterfaceContainer terminals_Interfaces;
	Ipv4InterfaceContainer p2pInterfaces;
	Ipv4InterfaceContainer apInterfaces;
	Ipv4AddressHelper address1;//记录子网，以便后续添加设备
	Ipv4AddressHelper address2;
	Ipv4AddressHelper address3;
}battalion;

//根据设置好的address为营分配ip
void Address_Assign(battalion * temp);

//将排级所有ip添加到营级的apInterfaces中
void Add_address(battalion * temp,platoon * plat);

//为所有的营级路由器和连级路由器和排级设备分配ip
void Ad_hoc_address(battalion *temp,company *Company,platoon * Platoon);

//初始化营级编制函数,传入参数为通信子网的wifi名称
battalion* initialize_battalion();
#endif

































