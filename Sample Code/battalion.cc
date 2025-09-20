#include "Global_variables.h"
#include "company.h"
#include "platoon.h"
#include "battalion.h"

using namespace ns3;

//营级编制，指挥控制子网+后勤支持子网+情报收集监控子网+通信子网p2p连接，营级通信子网通过路由器和连级通信子网交互
//根据设置好的address为营分配ip
void Address_Assign(battalion * temp)
{
	temp->terminals_Interfaces=temp->address1.Assign(temp->terminalDevices);
	temp->p2pInterfaces=temp->address2.Assign(temp->p2pDevices);
}

//将排级所有ip添加到营级的apInterfaces中
void Add_address(battalion * temp,platoon * plat)
{
	for(uint32_t i=0;i < plat->Interfaces.GetN();i++)
	{
		temp->apInterfaces.Add(plat->Interfaces.Get(i));
	}
}




//为所有的营级路由器和连级路由器和排级设备分配ip
void Ad_hoc_address(battalion *temp,company *Company,platoon * Platoon)
{
	//为营级路由器分配地址
	temp->apInterfaces=temp->address3.Assign(temp->apDevices);
	
	//为连路由器分配地址并加入营级网络设备中
	Company->apInterfaces=temp->address3.Assign(Company->apDevices);
	temp->apInterfaces.Add(Company->apInterfaces.Get(0));
	
	//为排所有终端设备分配地址并加入营级网络设备中
	Platoon->Interfaces=temp->address3.Assign(Platoon->terminalDevices);
	Add_address(temp,Platoon);
	
	/*
	for(uint32_t i=0;i<temp->apInterfaces.GetN();i++)
	{
		std::cout<<temp->apInterfaces.GetAddress(i)<<std::endl;
	}
	*/
}

//初始化营级编制函数,传入参数为通信子网的wifi名称
battalion* initialize_battalion(void)
{
	battalion * temp=new battalion();
	//terminals中9个节点，0为指挥控制，1为后勤支持，2为情报收集，3为通信
	temp->terminals.Create(4);
	//交换机
	temp->switches.Create(1);
	//路由器
	temp->router.Create(1);
	//----------------------------------------------------------------创建csma链路---------------------------------------
	CsmaHelper csma;
	//设置数据传输率
	csma.SetChannelAttribute("DataRate",StringValue("100Mbps"));
	//设置时延为6560纳秒，即6.56微秒
	csma.SetChannelAttribute("Delay",TimeValue(NanoSeconds(6560)));
	
	
	NodeContainer flag;
	for(uint32_t i=0;i < temp->terminals.GetN();i++)
	{
		flag=NodeContainer();
		flag.Add(temp->terminals.Get(i));
		flag.Add(temp->switches.Get(0));
		//为每一个终端安装csma链路,并记录到终端容器和交换机容器中
		NetDeviceContainer link;
		link=csma.Install(flag);
		temp->terminalDevices.Add(link.Get(0));
		temp->switchDevices.Add(link.Get(1));
	}
	
	//在交换机上安装桥接设备
	BridgeHelper bridge;
	NetDeviceContainer bridgeDevice = bridge.Install(temp->switches.Get(0), temp->switchDevices);
	
	//--------------------------------------------------------------------链接通信服务器和路由器---------------------------------------------
	PointToPointHelper p2p;
	//设置数据传输率
	p2p.SetDeviceAttribute("DataRate",StringValue("5Mbps"));
	//设置时延
	p2p.SetChannelAttribute("Delay",StringValue("2ms"));
	
	for(uint32_t i=0;i < temp->terminals.GetN();i++)
	{
		flag=NodeContainer();
		flag.Add(temp->terminals.Get(i));
		flag.Add(temp->switches.Get(0));
		//为每一个终端安装p2p链路,并记录到终端容器和交换机容器中
		NetDeviceContainer link;
		link=p2p.Install(flag);
		temp->terminalp2pDevices.Add(link.Get(0));
		temp->switchDevices.Add(link.Get(1));
	}
	
	flag=NodeContainer();
	flag.Add(temp->terminals.Get(3));
	flag.Add(temp->router.Get(0));
	temp->p2pDevices=p2p.Install(flag);
	
 	csma.EnablePcap("second", temp->terminalDevices.Get(3), true);
	//----------------------------------------------------------------------------给路由器安装无线链路-------------------------------
	//将前面设置好的phy、mac属性安装到节点容器nodes中，并返回到设备容器device中
	temp->apDevices = ns3::wifi.Install(ns3::phy, ns3::mac,temp->router);
	
	//-----------------------------------------------------------------------------------------设置移动模型--------------------------------------
	MobilityHelper mobility;
	//定点模型
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	//安装模型
	mobility.Install(temp->terminals);
	mobility.Install(temp->switches);
	mobility.Install(temp->router);
	
	//---------------------------------------------------------------------------------------------安装协议栈-----------------------------------
	InternetStackHelper stack;
	AodvHelper aodv;
	stack.SetRoutingHelper(aodv);  // 使用AODV作为路由协议
	stack.Install(temp->terminals);
	stack.Install(temp->switches);
	stack.Install(temp->router);
	
	return temp;
}

































