#include "platoon.h"
#include "Global_variables.h"

using namespace ns3;

//根据传入的数据初始化platoon中的数据
void initialize_range(platoon * temp,double minxposition,double maxxposition,double minyposition,double maxyposition)
{
	temp->minxposition=minxposition;
	temp->maxxposition=maxxposition;
	temp->minyposition=minyposition;
	temp->maxyposition=maxyposition;
}
/*
//为所有终端节点配置静态路由
void set_route(platoon *temp)
{
	Ptr<Ipv4StaticRouting> staticRouting1 = Ipv4RoutingHelper::GetRouting<Ipv4StaticRouting>(nodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol());
	staticRouting1->AddNetworkRouteTo(Ipv4Address("10.1.2.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("10.1.1.2"), 1);  // 添加静态路由

}
*/

//根据传入参数初始化移动模型
void set_model1(platoon *temp,double minspeed,double maxspeed,double flag)
{
	//无人机在z=20的平面运动，其他在地面运动
	//随机分配节点位置助手
	auto positionAllocator = CreateObject<RandomBoxPositionAllocator>();
	//设置分配位置的范围
	auto positionStream1 = CreateObject<UniformRandomVariable>();
	//设置x最小值和最大值
	positionStream1->SetAttribute("Min", DoubleValue(temp->minxposition));
	positionStream1->SetAttribute("Max", DoubleValue(temp->maxxposition));
	//将这个范围通知到助手
	positionAllocator->SetX(positionStream1);//横向范围
	
	auto positionStream2 = CreateObject<UniformRandomVariable>();
	//设置y最小值和最大值
	positionStream2->SetAttribute("Min", DoubleValue(temp->minyposition));
	positionStream2->SetAttribute("Max", DoubleValue(temp->maxyposition));
	
	positionAllocator->SetY(positionStream2);//纵向范围
	
	
	//确认助手分配位置的高度为Z=0，表示所有节点在Z=0的平面上移动
	positionAllocator->SetAttribute("Z", StringValue("ns3::ConstantRandomVariable[Constant=0.0])"));
	//设置分配速度的助手
	auto velocityStream = CreateObject<UniformRandomVariable>();
	//设置速度的范围
	velocityStream->SetAttribute("Min", DoubleValue(minspeed));
	velocityStream->SetAttribute("Max", DoubleValue(maxspeed));
	//创建模型助手
	MobilityHelper mobility;
	//指定移动模型参数
	mobility.SetMobilityModel(
		"ns3::RandomDirection2dMobilityModel",//2d平面内随机方向移动
		"Bounds",//设置移动边界
		RectangleValue({static_cast<double>(temp->minxposition),static_cast<double>(temp->maxxposition),static_cast<double>(temp->minyposition),static_cast<double>(temp->maxyposition)}),//分别设置横向和纵向的最大最小值
		"Speed",//设置速度属性
		PointerValue(velocityStream),//设置移动模型为之前设置好的速度分配器
		"Pause",//节点在改变方向时暂停时间
		StringValue("ns3::ConstantRandomVariable[Constant=1.0]")//每到达一个方向时间停止1秒钟
	);
	//为模型设置初始值/初始位置
	mobility.SetPositionAllocator(positionAllocator);
	//将该移动模型安装到节点/节点容器中
	//mobility.Install(temp->drones);
	mobility.Install(temp->servers);
	mobility.Install(temp->phones);
	mobility.Install(temp->drones.Get(1));
	//确认助手分配位置的高度为Z=40，表示所有节点在Z=40的平面上移动
	positionAllocator->SetAttribute("Z", StringValue("ns3::ConstantRandomVariable[Constant=40.0])"));
	mobility.SetPositionAllocator(positionAllocator);
	mobility.Install(temp->uavs.Get(1));
	mobility.Install(temp->uavs.Get(2));

	//--------------------------------------------------给簇头分配的位置近一点-------------------------------------
	positionStream1->SetAttribute("Min", DoubleValue(temp->minxposition));
	positionStream1->SetAttribute("Max", DoubleValue((3*temp->minxposition+temp->maxxposition)/4));
	//设置y最小值和最大值
	positionStream2->SetAttribute("Min", DoubleValue(temp->minyposition));
	positionStream2->SetAttribute("Max", DoubleValue((3*temp->minyposition+temp->maxyposition)/4));
	//将这个范围通知到助手
	positionAllocator->SetX(positionStream1);//横向范围
	positionAllocator->SetY(positionStream2);//纵向范围
	//指定移动模型参数
	mobility.SetMobilityModel(
		"ns3::RandomDirection2dMobilityModel",//2d平面内随机方向移动
		"Bounds",//设置移动边界
		RectangleValue({static_cast<double>(temp->minxposition),static_cast<double>((3*temp->minxposition+temp->maxxposition)/4),static_cast<double>(temp->minyposition),static_cast<double>((3*temp->minyposition+temp->maxyposition)/4)}),//分别设置横向和纵向的最大最小值
		"Speed",//设置速度属性
		PointerValue(velocityStream),//设置移动模型为之前设置好的速度分配器
		"Pause",//节点在改变方向时暂停时间
		StringValue("ns3::ConstantRandomVariable[Constant=1.0]")//每到达一个方向时间停止1秒钟
	);
	positionAllocator->SetAttribute("Z", StringValue("ns3::ConstantRandomVariable[Constant=0.0])"));
	//为模型设置初始值/初始位置
	mobility.SetPositionAllocator(positionAllocator);
	mobility.Install(temp->drones.Get(0));
	//----------------------------------------------------------为无人机0分配移动模型，移动路径为手动设置-------------------------
	//定点模型
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	//安装模型
	mobility.Install(temp->uavs.Get(0));
	if(flag==0.0)
	//设置定点位置
	temp->uavs.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(50, 170, 40)); // 节点坐标
	else
	temp->uavs.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(100, 195, 40)); // 节点坐标
}

//根据传入参数初始化移动模型
void set_model2(platoon *temp,double minspeed,double maxspeed)
{
	//无人机在z=20的平面运动，其他在地面运动
	//随机分配节点位置助手
	auto positionAllocator = CreateObject<RandomBoxPositionAllocator>();
	//设置分配位置的范围
	auto positionStream1 = CreateObject<UniformRandomVariable>();
	//设置x最小值和最大值
	positionStream1->SetAttribute("Min", DoubleValue(temp->minxposition));
	positionStream1->SetAttribute("Max", DoubleValue(temp->maxxposition));
	//将这个范围通知到助手
	positionAllocator->SetX(positionStream1);//横向范围
	
	auto positionStream2 = CreateObject<UniformRandomVariable>();
	//设置y最小值和最大值
	positionStream2->SetAttribute("Min", DoubleValue(temp->minyposition));
	positionStream2->SetAttribute("Max", DoubleValue(temp->maxyposition));
	
	positionAllocator->SetY(positionStream2);//纵向范围
	
	
	//确认助手分配位置的高度为Z=0，表示所有节点在Z=0的平面上移动
	positionAllocator->SetAttribute("Z", StringValue("ns3::ConstantRandomVariable[Constant=0.0])"));
	//设置分配速度的助手
	auto velocityStream = CreateObject<UniformRandomVariable>();
	//设置速度的范围
	velocityStream->SetAttribute("Min", DoubleValue(minspeed));
	velocityStream->SetAttribute("Max", DoubleValue(maxspeed));
	//创建模型助手
	MobilityHelper mobility;
	//指定移动模型参数
	mobility.SetMobilityModel(
		"ns3::RandomDirection2dMobilityModel",//2d平面内随机方向移动
		"Bounds",//设置移动边界
		RectangleValue({static_cast<double>(temp->minxposition),static_cast<double>(temp->maxxposition),static_cast<double>(temp->minyposition),static_cast<double>(temp->maxyposition)}),//分别设置横向和纵向的最大最小值
		"Speed",//设置速度属性
		PointerValue(velocityStream),//设置移动模型为之前设置好的速度分配器
		"Pause",//节点在改变方向时暂停时间
		StringValue("ns3::ConstantRandomVariable[Constant=1.0]")//每到达一个方向时间停止1秒钟
	);
	//为模型设置初始值/初始位置
	mobility.SetPositionAllocator(positionAllocator);
	//将该移动模型安装到节点/节点容器中
	//mobility.Install(temp->drones);
	mobility.Install(temp->servers);
	mobility.Install(temp->phones);
	mobility.Install(temp->drones.Get(1));
	//确认助手分配位置的高度为Z=40，表示所有节点在Z=40的平面上移动
	positionAllocator->SetAttribute("Z", StringValue("ns3::ConstantRandomVariable[Constant=40.0])"));
	mobility.SetPositionAllocator(positionAllocator);
	mobility.Install(temp->uavs);

	//--------------------------------------------------给簇头分配的位置近一点-------------------------------------
	positionStream1->SetAttribute("Min", DoubleValue(temp->minxposition));
	positionStream1->SetAttribute("Max", DoubleValue((3*temp->minxposition+temp->maxxposition)/4));
	//设置y最小值和最大值
	positionStream2->SetAttribute("Min", DoubleValue(temp->minyposition));
	positionStream2->SetAttribute("Max", DoubleValue((3*temp->minyposition+temp->maxyposition)/4));
	//将这个范围通知到助手
	positionAllocator->SetX(positionStream1);//横向范围
	positionAllocator->SetY(positionStream2);//纵向范围
	//指定移动模型参数
	mobility.SetMobilityModel(
		"ns3::RandomDirection2dMobilityModel",//2d平面内随机方向移动
		"Bounds",//设置移动边界
		RectangleValue({static_cast<double>(temp->minxposition),static_cast<double>((3*temp->minxposition+temp->maxxposition)/4),static_cast<double>(temp->minyposition),static_cast<double>((3*temp->minyposition+temp->maxyposition)/4)}),//分别设置横向和纵向的最大最小值
		"Speed",//设置速度属性
		PointerValue(velocityStream),//设置移动模型为之前设置好的速度分配器
		"Pause",//节点在改变方向时暂停时间
		StringValue("ns3::ConstantRandomVariable[Constant=1.0]")//每到达一个方向时间停止1秒钟
	);
	positionAllocator->SetAttribute("Z", StringValue("ns3::ConstantRandomVariable[Constant=0.0])"));
	//为模型设置初始值/初始位置
	mobility.SetPositionAllocator(positionAllocator);
	mobility.Install(temp->drones.Get(0));
}

//初始化排级编制函数,传入参数为通信子网的wifi名称
platoon* initialize_platoon(void)
{
	platoon * temp=new platoon();
	temp->drones.Create(2);
	temp->uavs.Create(3);
	temp->phones.Create(3);
	temp->servers.Create(2);
	//-------------------------------------------------------------------------------------安装网络栈-------------------------------------------------
	InternetStackHelper stack;
	AodvHelper aodv;
	stack.SetRoutingHelper(aodv);  // 使用AODV作为路由协议
	stack.Install(temp->drones);
	stack.Install(temp->uavs);
	stack.Install(temp->phones);
	stack.Install(temp->servers);
	//----------------------------------------------------------------------------------------设置为无线模式---------------------------------------
	//将前面设置好的phy、mac属性安装到节点容器中，并返回到设备容器device中
	temp->terminalDevices =ns3::wifi.Install(ns3::phy,ns3::mac, temp->drones);
	temp->terminalDevices.Add(ns3::wifi.Install(ns3::phy, ns3::mac,temp->phones));
	temp->terminalDevices.Add(ns3::wifi.Install(ns3::phy, ns3::mac,temp->uavs));
	temp->terminalDevices.Add(ns3::wifi.Install(ns3::phy, ns3::mac,temp->servers));
	return temp;
}

/*
//更改单个无人机的移动模型
void modifyposition(platoon *temp,double minspeed,double maxspeed)
{
	//无人机在z=20的平面运动，其他在地面运动
	//随机分配节点位置助手
	auto positionAllocator = CreateObject<RandomBoxPositionAllocator>();
	//设置分配位置的范围
	auto positionStream1 = CreateObject<UniformRandomVariable>();
	//设置x最小值和最大值
	positionStream1->SetAttribute("Min", DoubleValue(temp->minxposition));
	positionStream1->SetAttribute("Max", DoubleValue(temp->maxxposition));
	//将这个范围通知到助手
	positionAllocator->SetX(positionStream1);//横向范围
	
	auto positionStream2 = CreateObject<UniformRandomVariable>();
	//设置y最小值和最大值
	positionStream2->SetAttribute("Min", DoubleValue(temp->minyposition));
	positionStream2->SetAttribute("Max", DoubleValue(temp->maxyposition));
	
	positionAllocator->SetY(positionStream2);//纵向范围
	
	
	//确认助手分配位置的高度为Z=0，表示所有节点在Z=0的平面上移动
	positionAllocator->SetAttribute("Z", StringValue("ns3::ConstantRandomVariable[Constant=0.0])"));
	//设置分配速度的助手
	auto velocityStream = CreateObject<UniformRandomVariable>();
	//设置速度的范围
	velocityStream->SetAttribute("Min", DoubleValue(minspeed));
	velocityStream->SetAttribute("Max", DoubleValue(maxspeed));
	//创建模型助手
	MobilityHelper mobility;
	//指定移动模型参数
	mobility.SetMobilityModel(
		"ns3::RandomDirection2dMobilityModel",//2d平面内随机方向移动
		"Bounds",//设置移动边界
		RectangleValue({static_cast<double>(temp->minxposition),static_cast<double>(temp->maxxposition),static_cast<double>(temp->minyposition),static_cast<double>(temp->maxyposition)}),//分别设置横向和纵向的最大最小值
		"Speed",//设置速度属性
		PointerValue(velocityStream),//设置移动模型为之前设置好的速度分配器
		"Pause",//节点在改变方向时暂停时间
		StringValue("ns3::ConstantRandomVariable[Constant=1.0]")//每到达一个方向时间停止1秒钟
	);
	//为模型设置初始值/初始位置
	mobility.SetPositionAllocator(positionAllocator);
	//将该移动模型安装到节点/节点容器中
	//mobility.Install(temp->drones);
	mobility.Install(temp->servers);
	mobility.Install(temp->phones);
	mobility.Install(temp->drones.Get(1));
	//确认助手分配位置的高度为Z=40，表示所有节点在Z=40的平面上移动
	positionAllocator->SetAttribute("Z", StringValue("ns3::ConstantRandomVariable[Constant=40.0])"));
	mobility.SetPositionAllocator(positionAllocator);
	mobility.Install(temp->uavs.Get(1));
	mobility.Install(temp->uavs.Get(2));

	//给簇头分配的位置近一点
	positionStream1->SetAttribute("Min", DoubleValue(temp->minxposition));
	positionStream1->SetAttribute("Max", DoubleValue((3*temp->minxposition+temp->maxxposition)/4));
	//设置y最小值和最大值
	positionStream2->SetAttribute("Min", DoubleValue(temp->minyposition));
	positionStream2->SetAttribute("Max", DoubleValue((3*temp->minyposition+temp->maxyposition)/4));
	//将这个范围通知到助手
	positionAllocator->SetX(positionStream1);//横向范围
	positionAllocator->SetY(positionStream2);//纵向范围
	//指定移动模型参数
	mobility.SetMobilityModel(
		"ns3::RandomDirection2dMobilityModel",//2d平面内随机方向移动
		"Bounds",//设置移动边界
		RectangleValue({static_cast<double>(temp->minxposition),static_cast<double>((3*temp->minxposition+temp->maxxposition)/4),static_cast<double>(temp->minyposition),static_cast<double>((3*temp->minyposition+temp->maxyposition)/4)}),//分别设置横向和纵向的最大最小值
		"Speed",//设置速度属性
		PointerValue(velocityStream),//设置移动模型为之前设置好的速度分配器
		"Pause",//节点在改变方向时暂停时间
		StringValue("ns3::ConstantRandomVariable[Constant=1.0]")//每到达一个方向时间停止1秒钟
	);
	positionAllocator->SetAttribute("Z", StringValue("ns3::ConstantRandomVariable[Constant=0.0])"));
	//为模型设置初始值/初始位置
	mobility.SetPositionAllocator(positionAllocator);
	mobility.Install(temp->drones.Get(0));
	
	//-----------------------------------------------------更改无人机路径----------------
	double minpositionx=150.0,maxpositionx=200.0;
	double minpositiony=-10.0,maxpositiony=10.0;
	//无人机在z=20的平面运动，其他在地面运动
	//设置x最小值和最大值
	positionStream1->SetAttribute("Min", DoubleValue(minpositionx));
	positionStream1->SetAttribute("Max", DoubleValue(maxpositionx));
	//将这个范围通知到助手
	positionAllocator->SetX(positionStream1);//横向范围
	
	//设置y最小值和最大值
	positionStream2->SetAttribute("Min", DoubleValue(minpositiony));
	positionStream2->SetAttribute("Max", DoubleValue(maxpositiony));
	
	positionAllocator->SetY(positionStream2);//纵向范围
	
	//创建模型助手
	MobilityHelper mobility1;
	//指定移动模型参数
	mobility1.SetMobilityModel(
		"ns3::RandomDirection2dMobilityModel",//2d平面内随机方向移动
		"Bounds",//设置移动边界
		RectangleValue({static_cast<double>(minpositionx),static_cast<double>(maxpositionx),static_cast<double>(minpositiony),static_cast<double>(maxpositiony)}),//分别设置横向和纵向的最大最小值
		"Speed",//设置速度属性
		PointerValue(velocityStream),//设置移动模型为之前设置好的速度分配器
		"Pause",//节点在改变方向时暂停时间
		StringValue("ns3::ConstantRandomVariable[Constant=1.0]")//每到达一个方向时间停止1秒钟
	);
	//为模型设置初始值/初始位置
	mobility1.SetPositionAllocator(positionAllocator);
	//将该移动模型安装到节点/节点容器中
	//mobility.Install(temp->drones);
	//确认助手分配位置的高度为Z=40，表示所有节点在Z=40的平面上移动
	positionAllocator->SetAttribute("Z", StringValue("ns3::ConstantRandomVariable[Constant=40.0])"));
	mobility1.SetPositionAllocator(positionAllocator);
	mobility1.Install(temp->uavs.Get(0));
}

*/





























