#ifndef COMPANY_H
#define COMPANY_H

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

//连级编制
//7个teminal节点+1个交换机+1个路由器
typedef struct company
{
	NodeContainer terminals;
	NodeContainer switches;
	NodeContainer router;
	//设置网络设备用于分配ip
	NetDeviceContainer terminalDevices,switchDevices,p2pDevices,apDevices,terminalp2pDevices;
	//设置子网容器，能够获取他们的ip
	Ipv4InterfaceContainer terminals_Interfaces;//记录csma分配的ip地址
	Ipv4InterfaceContainer p2pInterfaces;//记录p2p链路地址
	Ipv4InterfaceContainer apInterfaces;//记录Ad_hoc地址
	Ipv4AddressHelper address1;
	Ipv4AddressHelper address2;
}company;

//根据设置好的address为所有设备分配ip
void Assign_Company_Address(company* temp);

//初始化连级编制函数,传入参数为通信子网的wifi名称
company* initialize_company();
#endif

































