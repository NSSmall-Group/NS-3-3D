#include "company.h"
#include "Global_variables.h"

using namespace ns3;

//根据设置好的address为所有设备分配ip
void Assign_Company_Address(company* temp)
{
	temp->terminals_Interfaces=temp->address1.Assign(temp->terminalDevices);
	temp->p2pInterfaces=temp->address2.Assign(temp->p2pDevices);
}

//初始化连级编制函数,传入参数为通信子网的wifi名称
company* initialize_company(void)
{
	company * temp=new company();
	//0为指挥控制子网，1为情报收集监控子网，2为通信子网
	temp->terminals.Create(3);
	temp->switches.Create(1);
	temp->router.Create(1);
	//---------------------------------------------------------------------------为七个终端与交换机之间创建csma链路---------------------------------------
	//设置有线助手
	CsmaHelper csma;
	//设置数据传输率
	csma.SetChannelAttribute("DataRate",StringValue("100Mbps"));
	//设置时延为6560纳秒，即6.56微秒
	csma.SetChannelAttribute("Delay",TimeValue(NanoSeconds(6560)));
	NodeContainer flag;
	for(uint32_t i = 0; i<temp->terminals.GetN();i++)
	{
		flag=NodeContainer();
		flag.Add(temp->terminals.Get(i));
		flag.Add(temp->switches.Get(0));
		//链接好后安装csma，并用temp->terminalDevices和temp->switchDevices保存设备
		NetDeviceContainer link;
		link=csma.Install(flag);
		temp->terminalDevices.Add(link.Get(0));
		temp->switchDevices.Add(link.Get(1));
	}
	//在交换机上安装桥接设备
	BridgeHelper bridge;
	NetDeviceContainer bridgeDevice = bridge.Install(temp->switches.Get(0), temp->switchDevices);
	//--------------------------------------------------------------------------------------链接通信服务器和路由器------------------------------------------
	PointToPointHelper p2p;
	//设置数据传输率
	p2p.SetDeviceAttribute("DataRate",StringValue("5Mbps"));
	//设置时延
	p2p.SetChannelAttribute("Delay",StringValue("2ms"));
	for(uint32_t i = 0; i<temp->terminals.GetN();i++)
	{
		flag=NodeContainer();
		flag.Add(temp->terminals.Get(i));
		flag.Add(temp->switches.Get(0));
		//链接好后安装csma，并用temp->terminalDevices和temp->switchDevices保存设备
		NetDeviceContainer link;
		link=p2p.Install(flag);
		temp->terminalp2pDevices.Add(link.Get(0));
		temp->switchDevices.Add(link.Get(1));
	}
	flag=NodeContainer();
	flag.Add(temp->terminals.Get(2));
	flag.Add(temp->router.Get(0));
	temp->p2pDevices=p2p.Install(flag);
	
	//-----------------------------------------------------------------------------------------给路由器安装局域网-------------------------------------------
	//将前面设置好的phy、mac属性安装到节点容器nodes中，并返回到设备容器device中
	temp->apDevices =ns3::wifi.Install(ns3::phy,ns3::mac,temp->router);
	
	//---------------------------------------------------------------------------------------------设置移动模型-------------------------------------------------
	MobilityHelper mobility;
	//定点模型
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	//安装模型
	mobility.Install(temp->terminals);
	mobility.Install(temp->switches);
	mobility.Install(temp->router);
	
	//-----------------------------------------------------------------------------------------------安装协议栈--------------------------------------------------
	InternetStackHelper stack;
	AodvHelper aodv;
	stack.SetRoutingHelper(aodv);  // 使用AODV作为路由协议
	stack.Install(temp->terminals);
	stack.Install(temp->switches);
	stack.Install(temp->router);
	
	
	return temp;
}

































