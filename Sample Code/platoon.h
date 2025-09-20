#ifndef PLATOON_H
#define PLATOON_H

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
#include "company.h"
#include "ns3/aodv-helper.h"
#include "ns3/wifi-module.h"


using namespace ns3;

/*
control_subnet:指挥控制子网
support_subnet:后勤支持子网
collection_subnet:情报收集子网
communication_subnet:通信子网
*/

//排级编制
//1小车+1个背负台+3个手持设备+2个无人机
typedef struct platoon
{
	//移动范围
	int minxposition;
	int minyposition;
	int maxxposition;
	int maxyposition;
	NodeContainer drones;
	NodeContainer servers;
	NodeContainer phones;
	NodeContainer uavs;
	//设置网络设备用于分配ip
	NetDeviceContainer terminalDevices;
	//设置子网容器，能够获取他们的ip
	Ipv4InterfaceContainer Interfaces;//记录Ad_hoc地址
}platoon;

//根据传入的数据初始化platoon中的数据
void initialize_range(platoon * temp,double minxposition,double maxxposition,double minyposition,double maxyposition);

//根据传入参数初始化移动模型
void set_model1(platoon *temp,double minspeed,double maxspeed,double flag);
void set_model2(platoon *temp,double minspeed,double maxspeed);

//初始化排级编制函数,传入参数为通信子网的wifi名称
platoon* initialize_platoon();
//更改单个无人机路径
//void modifyposition(platoon *temp,double minspeed,double maxspeed);
#endif

































