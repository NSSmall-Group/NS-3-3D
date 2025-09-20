#include "battalion.h"
#include "Global_variables.h"
#include "ns3/netanim-module.h"

//日志输出，采用*eventLog<<Simulator...方式直接写入Netsimulyzer
Ptr<netsimulyzer::LogStream> eventLog;
//设置日志组件，可以在ns3控制台设置不同的级别来查看日志
NS_LOG_COMPONENT_DEFINE ("mydemo");

double sendtime=1.0;
double receivetime=0.5;


void PacketSendLogger (Ptr<const Packet> packet) {
    *eventLog << Simulator::Now().GetSeconds() << "s: Packet sent, size: " << packet->GetSize() << " bytes\n";
}

void PacketReceiveLogger (Ptr<const Packet> packet) {
    *eventLog << Simulator::Now().GetSeconds() << "s: Packet received, size: " << packet->GetSize() << " bytes\n";
}

/*
void HideNodeModel(netsimulyzer::NodeConfigurationHelper *nodeConfigHelper,Ptr<Node> node) {
	std::cout << "Hiding node model at time: " << Simulator::Now().GetSeconds() << "s" << std::endl;
	nodeConfigHelper->Set("EnableMotionTrail", BooleanValue(false)); // 设置模型不可见
	nodeConfigHelper->Install(node);
}
*/

void UpdatePosition(Ptr<Node> node, Vector position) {
    Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
    mobility->SetPosition(position);
    std::cout << "Updated position to: " << position << std::endl;
}

void MySendFunction(Ptr<Node> node,double data)
{
	Ptr<netsimulyzer::NodeConfiguration> nodeConfig = node->GetObject<netsimulyzer::NodeConfiguration>();
	if (nodeConfig != nullptr) {
            // Display a transmission animation for each node
            nodeConfig->Transmit(Seconds(sendtime), data, netsimulyzer::GRAY_30);
        }
}

void MyReceiveFunction(Ptr<Node> node,double data)
{
	Ptr<netsimulyzer::NodeConfiguration> nodeConfig = node->GetObject<netsimulyzer::NodeConfiguration>();
	if (nodeConfig != nullptr) {
            // Display a transmission animation for each node
            nodeConfig->Transmit(Seconds(receivetime), data, netsimulyzer::RED);
        }
}


void
CourseChanged(Ptr<const MobilityModel> model)
{
    //auto 关键字用于在声明变量时自动推导变量的类型。编译器会根据变量的初始化值来确定其类型
    //获取节点对象：model->GetObject<Node>() 返回与该移动模型关联的节点对象。
    //获取节点 ID：GetId() 方法返回该节点的唯一标识符（ID），并将其存储在 nodeId 变量中。
    const auto nodeId = model->GetObject<Node>()->GetId();
    //返回节点当前位置
    const auto position = model->GetPosition();
    //返回节点当前速度向量
    const auto velocity = model->GetVelocity();
    //Simulator::Now().GetMilliSeconds()获取当前的模拟时间，以毫秒为单位
    //<<操作符用于将信息格式化并写入日志流
    /*
    position.x, position.y, position.z 和 velocity.x, velocity.y, velocity.z 用于输出节点的新位置和速度。
    */
    *eventLog << Simulator::Now().GetMilliSeconds() << ": Node [" << nodeId
              << "] Course Change Position: [" << position.x << ", " << position.y << ", "
              << position.z << "] "
              << "Velocity [" << velocity.x << ", " << velocity.y << ", " << velocity.z << "]\n";
}

int main(int argc,char *argv[])
{
	double minposition=-100;
	double maxposition=100;
	double minspeed=10;
	double maxspeed=10;
	double duration=100;
	//获取netsimulyzer版本信息
	std::cout << "Netsimulyzer version: " << ns3::netsimulyzer::versionString() << std::endl;
	
	InitializeWifiSettings();
	
	//设置输出文件名称
	std::string outputFileName = "XIDIAN_two_scene.json";
	
	//最后的文件名router全部小写要大写
	std::string routerPath=netsimulyzer::models::ROUTER;//路由器/交换机
	std::string serverPath=netsimulyzer::models::SERVER;//服务器
	std::string laptopPath=netsimulyzer::models::LAPTOP;//背负台
	std::string phonePath=netsimulyzer::models::SMARTPHONE;//手持设备
	std::string uavPath=netsimulyzer::models::QUADCOPTER_UAV;//无人机
	std::string dronePath=netsimulyzer::models::LAND_DRONE;//小车
	std::string tower=netsimulyzer::models::CELL_TOWER;//基站
	
	battalion * Battalion=initialize_battalion();//营
	company * Company=initialize_company();//连
	platoon * Platoon=initialize_platoon();//排
	platoon * Platoon1=initialize_platoon();//2排
	//设置命令行
	CommandLine cmd;
	//选择可以从命令行读取哪些参数
	//例如，移动范围，速度范围，仿真时长
	cmd.AddValue("minposition","Minimum X/Y position a Node may move to",minposition);
	cmd.AddValue("maxposition","Maximum X/Y position a Node may move to",maxposition);
	cmd.AddValue("minspeed","the Minimum speed that a Node may move",minspeed);
	cmd.AddValue("maxspeed","the Maximum speed that a Node may move",maxspeed);
	cmd.AddValue("outputFileName","The name of the file to write the NetSimulyzer trace info",outputFileName);
        cmd.AddValue("duration", "Duration (in Seconds) of the simulation", duration);
        //指定无人小车的模型路径            
	cmd.AddValue("dronePath","The path to the model file to represent the drone Nodes",dronePath);
	//指定手机的模型路径
	cmd.AddValue("phonePath","The path to the model file to represent the Phone Nodes",phonePath);
	//指定无人机的模型路径
	cmd.AddValue("uavPath","The path to the model file to represent the Uav Nodes",uavPath);
	//指定服务器的模型路径
	cmd.AddValue("serverPath","The path to the model file to represent the Server Nodes",serverPath);
	//指定路由器的模型路径
	cmd.AddValue("routerPath","The path to the model file to represent the Router Nodes",routerPath);
	//指定背负台的模型路径
	cmd.AddValue("laptopPath","The path to the model file to represent the Laptop Nodes",laptopPath);
	//agrc表示参数个数，命令行什么都不输入默认argc为1，argv为指向参数的指针
	cmd.Parse(argc,argv);
	//该命令会在参数满足特定条件时终止程序，并打印错误信息
	NS_ABORT_MSG_IF(duration < 1.0, "仿真时间过短！" );
	
	initialize_range(Platoon,minposition,maxposition,20,2*maxposition+20);
	initialize_range(Platoon1,minposition,maxposition,2*minposition-20,-20);
	//设置移动模型
	//modifyposition(Platoon,minspeed,maxspeed);
	double flag=0;
	set_model1(Platoon,minspeed,maxspeed,flag);
	set_model2(Platoon1,minspeed,maxspeed);
	//分配ip
	//为营分配ip
	Battalion->address1.SetBase("192.168.1.0","255.255.255.0");
	Battalion->address2.SetBase("192.168.2.0","255.255.255.0");
	Address_Assign(Battalion);
	
	//为连分配ip
	Company->address1.SetBase("192.168.3.0","255.255.255.0");
	Company->address2.SetBase("192.168.4.0","255.255.255.0");
	Assign_Company_Address(Company);
	
	//为所有Ad_hoc设备分配ip
	Battalion->address3.SetBase("10.0.1.0","255.255.255.0");
	Ad_hoc_address(Battalion,Company,Platoon);
	Ad_hoc_address(Battalion,Company,Platoon1);
	
	
	//全局路由
	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	
	//初始化所有节点的位置
	//初始化营位置

	Battalion->terminals.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-120, -40.0, 0.0)); // 节点坐标
	Battalion->terminals.Get(1)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-140, 0.0, 0.0)); // 节点坐标
	Battalion->terminals.Get(2)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-120, 40.0, 0.0)); // 节点坐标
	Battalion->terminals.Get(3)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-100, 0.0, 0.0)); // 节点坐标
	Battalion->switches.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-120, 0.0, 0.0)); // 节点坐标
	Battalion->router.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-80, 0.0, 0.0)); // 节点坐标
	//初始化连位置

	Company->terminals.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-50,minposition-10,0.0));
	Company->terminals.Get(1)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-10,minposition+30,0.0));
	Company->terminals.Get(2)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-50,minposition+30,0.0));
	Company->switches.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-50,minposition+10,0.0));
	Company->router.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-50,minposition+50,0.0));
	
	//------------------------------------------------------------安装udp客户机/服务器---------------------------------
	//设置udp服务器助手，收发端口为9
	UdpEchoServerHelper echoServer(9);
	//将服务器安装到通信双方其中之一上
	ApplicationContainer serverApps = echoServer.Install(Battalion->terminals.Get(0));
	//设置服务器的运行开始与结束的时间
	serverApps.Start(Seconds(1.0));
	serverApps.Stop(Seconds(10.0));
	//设置udp客户机助手，告诉客户机助手服务器的ip地址与端口
	UdpEchoClientHelper echoClient(Battalion->terminals_Interfaces.GetAddress(0), 9);
	//设置回显请求参数
	//设置发送回显请求个数
	echoClient.SetAttribute("MaxPackets", UintegerValue(1));
	//设置每两个回显请求时间发送间隔
	echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
	//回显请求大小
	echoClient.SetAttribute("PacketSize", UintegerValue(1024));

	//将客户机安装到两个通信设备中的另一台设备上
	ApplicationContainer clientApps = echoClient.Install(Platoon->drones.Get(0));

	//设置客户机的运行时间
	clientApps.Start(Seconds(2.0));
	clientApps.Stop(Seconds(10.0));

	//以LOG_INFO级别输出udp日志
	LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
	
	//----------------------------------------------------------------仿真------------------------------------------
	//iter=NodeList::Begin()获取节点列表中的第一个节点
	//NodeList::End()用于获取最后一个节点的下一个位置
	//iter++获取下一个节点位置
	for (auto iter = NodeList::Begin(); iter != NodeList::End(); iter++)
	    {
		//获取节点的移动模型
		auto m = (*iter)->GetObject<MobilityModel>();
		//判断节点是否安装了移动模型，如果m为nullptr，则m没有附加移动模型
		if (!m)
		    continue;
		//连接轨迹回调
		//这一行代码将移动性模型的"CourseChange"事件连接到回调函数CourseChanged
		//回调函数在移动性模型的路径或未知发生变化时被触发
		//MakeCallback(&CourseChanged)用于将CourseChanged函数作为回调传递
		m->TraceConnectWithoutContext("CourseChange", MakeCallback(&CourseChanged));
	    }

	// 设置回调函数，用于捕捉发送和接收事件
	//记录客户机发送数据包
	clientApps.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback(&PacketSendLogger));
	//记录服务器接收数据包
	serverApps.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&PacketReceiveLogger));
	//记录服务器发送数据包
	serverApps.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback(&PacketSendLogger));
	//记录客户机接收数据包
	clientApps.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&PacketReceiveLogger));
	
	//netsimulyzer中的Orchestrator对象用于管理可视化场景，并输出记录到指定的JSON文件中
	//--------------------------------------------Orchestrator对象用于管理和协调仿真中的所有可视化组件----------------
	auto orchestrator = CreateObject<netsimulyzer::Orchestrator>(outputFileName);
	//设置场景为矩形
	auto possibleNodeLocations = CreateObject<netsimulyzer::RectangularArea>(
		orchestrator,
		Rectangle{minposition, maxposition, 20, 2*maxposition+20});
	//为区域设置名称
	possibleNodeLocations->SetAttribute("Name", StringValue("XiDian University"));
	//为区域设置颜色
	//SetAttribute("FillColor", ...)：设置矩形区域的填充颜色为浅绿色。
	//Color3Value 是一个表示颜色的结构体，使用 RGB 颜色模型。
	possibleNodeLocations->SetAttribute("FillColor", ns3::netsimulyzer::Color3Value{204u, 255u, 204u});
	
	auto possibleNodeLocations1 = CreateObject<netsimulyzer::RectangularArea>(
		orchestrator,
		Rectangle{minposition, maxposition, 2*minposition-20, -20});
	//为区域设置名称
	possibleNodeLocations1->SetAttribute("Name", StringValue("XiDian University"));
	//为区域设置颜色
	//SetAttribute("FillColor", ...)：设置矩形区域的填充颜色为浅绿色。
	//Color3Value 是一个表示颜色的结构体，使用 RGB 颜色模型。
	possibleNodeLocations1->SetAttribute("FillColor", ns3::netsimulyzer::Color3Value{0u, 0u, 0u});
	
	//创建两个日志流对象 infoLog 和 eventLog，用于记录场景配置和事件信息
	//声明该日志流属于orchestrator可视化助手
	auto infoLog = CreateObject<netsimulyzer::LogStream>(orchestrator);
	eventLog = CreateObject<netsimulyzer::LogStream>(orchestrator);

	//记录场景配置，以下内容会被输出到netsimulyzer的控制台中
	*infoLog << "----- Scenario Settings -----\n";


	//记录移动范围
	*infoLog << "Node Position Range: [" << minposition << ',' << maxposition << "]\n";
	/*
	也可以这样输出
	*infoLog << "Node Position Range (X): [" << minXPosition << ", " << maxXPosition << "]\n"; 
	*infoLog << "Node Position Range (Y): [" << minYPosition << ", " << maxYPosition << "]\n";
	*/

	//记录速度范围
	*infoLog << "Node Speed Range: [" << minspeed << ',' << maxspeed << "]\n";
	//记录模型的存储位置
	*infoLog << "Models: Phone [" << phonePath << "], Drone [" << dronePath << "], Uav [" << uavPath << "], server [" << serverPath << "], router [" << routerPath << "], Laptop [" << laptopPath << "]\n";
	//记录仿真持续时间
	*infoLog << "Scenario Duration (Seconds): " << duration << '\n';	
	
	//为节点安装模型——>声明模型地址，根据前面的指定模型路径来设置

	//设置节点配置助手
	netsimulyzer::NodeConfigurationHelper nodeConfigHelper(orchestrator);

	//EnableMotionTrail：设置为 true，表示在可视化中启用节点的运动轨迹。
	nodeConfigHelper.Set("EnableMotionTrail", BooleanValue(true));
	//有多个模型就多次执行下面两行代码
	
	double BattalionScale=7;
	double CompanyScale=5;
	//ModelPath直接用前面设置好的droneModelPath或spaceship即可
	nodeConfigHelper.Set("Model",StringValue(serverPath));
	nodeConfigHelper.Set("Scale",ns3::DoubleValue(BattalionScale));
	nodeConfigHelper.Set("Name",ns3::StringValue("Class III"));
	nodeConfigHelper.Set ("HighlightColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::BLUE});
	//利用节点配置助手将模型安装到节点（容器）上
	//安装营级服务器
	nodeConfigHelper.Install(Battalion->terminals.Get(0));
	nodeConfigHelper.Install(Battalion->terminals.Get(1));
	nodeConfigHelper.Install(Battalion->terminals.Get(2));
	
	//安装连级服务器
	nodeConfigHelper.Set("Scale",ns3::DoubleValue(CompanyScale));
	nodeConfigHelper.Set ("HighlightColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::WHITE});
	nodeConfigHelper.Install(Company->terminals.Get(0));
	nodeConfigHelper.Install(Company->terminals.Get(1));
	
	//安装IV类设备
	nodeConfigHelper.Set("Name",ns3::StringValue("Class IV"));
	nodeConfigHelper.Set("Scale",ns3::DoubleValue(BattalionScale));
	nodeConfigHelper.Set ("HighlightColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::BLUE});
	nodeConfigHelper.Install(Battalion->terminals.Get(3));
	
	nodeConfigHelper.Set("Name",ns3::StringValue("Class IV"));
	nodeConfigHelper.Set("Scale",ns3::DoubleValue(CompanyScale));
	nodeConfigHelper.Set ("HighlightColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::WHITE});
	nodeConfigHelper.Install(Company->terminals.Get(2));
	
	
	//安装背负台
	nodeConfigHelper.Set("Model",StringValue(laptopPath));
	nodeConfigHelper.Set("Scale",ns3::DoubleValue(5.0));
	nodeConfigHelper.Set("Name",ns3::StringValue("Class I"));
	nodeConfigHelper.Set("HighlightColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::WHITE});
	nodeConfigHelper.Set ("MotionTrailColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::WHITE});
	nodeConfigHelper.Install(Platoon->servers);
	nodeConfigHelper.Install(Platoon1->servers);
	//安装路由器/交换机
	nodeConfigHelper.Set("Model",StringValue(routerPath));
	nodeConfigHelper.Set("Scale",ns3::DoubleValue(5.0));
	nodeConfigHelper.Set("Name",ns3::StringValue("Class II"));
	nodeConfigHelper.Set ("HighlightColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::BLUE});
	nodeConfigHelper.Set ("MotionTrailColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::WHITE});
	nodeConfigHelper.Install(Battalion->switches);
	nodeConfigHelper.Install(Company->switches);
	//架基站
	//nodeConfigHelper.Set("Scale",ns3::DoubleValue(0.5));
	//nodeConfigHelper.Set("Name",ns3::StringValue("Class II"));
	//nodeConfigHelper.Set("Model",StringValue(tower));
	nodeConfigHelper.Install(Battalion->router);
	nodeConfigHelper.Install(Company->router);
	
	//安装小车
	nodeConfigHelper.Set("Model",StringValue(dronePath));
	nodeConfigHelper.Set("Scale",ns3::DoubleValue(5.0));
	nodeConfigHelper.Set("Name",ns3::StringValue("Class I"));
	nodeConfigHelper.Set ("HighlightColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::BLUE});
	nodeConfigHelper.Set ("MotionTrailColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::RED});
	nodeConfigHelper.Install(Platoon->drones.Get(0));
	nodeConfigHelper.Set ("MotionTrailColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::WHITE});
	nodeConfigHelper.Install(Platoon->drones.Get(1));
	nodeConfigHelper.Install(Platoon1->drones);
	
	//安装无人机
	nodeConfigHelper.Set("Model",StringValue(uavPath));
	nodeConfigHelper.Set("Scale",ns3::DoubleValue(5.0));
	nodeConfigHelper.Set("Name",ns3::StringValue("Class I"));
	nodeConfigHelper.Set ("HighlightColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::BLUE});
	nodeConfigHelper.Set ("MotionTrailColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::WHITE});
	nodeConfigHelper.Install(Platoon->uavs);
	nodeConfigHelper.Install(Platoon1->uavs);
	
	//安装手机
	nodeConfigHelper.Set("Model",StringValue(phonePath));
	nodeConfigHelper.Set("Scale",ns3::DoubleValue(5.0));
	nodeConfigHelper.Set("Name",ns3::StringValue("Class I"));
	nodeConfigHelper.Set ("HighlightColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::WHITE});
	nodeConfigHelper.Set ("MotionTrailColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::WHITE});
	nodeConfigHelper.Install(Platoon->phones);
	nodeConfigHelper.Install(Platoon1->phones);
	
	/*
	// 创建一个 OutputStreamWrapper，将 std::cout 包装为 Ptr<OutputStreamWrapper>
	Ptr<OutputStreamWrapper> stream = Create<OutputStreamWrapper>(&std::cout);
	//在仿真时间1.0秒时打印所有节点的路由表
	Ipv4GlobalRoutingHelper::PrintRoutingTableAllAt(Seconds(1.0), stream);
	*/
	
	//发送到接收有0.5秒延迟
	//从接收到发送有1秒延迟
	//接收动画播放receivetime秒 == 0.5秒
	//发送动画播放sendtime秒 == 1.0 秒
	Simulator::Schedule(Seconds(2.0), &MySendFunction,Platoon->phones.Get(0),20.0);//终端发送
	Simulator::Schedule(Seconds(2.5), &MyReceiveFunction,Platoon->drones.Get(0),20.0);//簇头接收
	Simulator::Schedule(Seconds(2.5+receivetime),&MySendFunction,Platoon->drones.Get(0),30.0);//簇头发送
	Simulator::Schedule(Seconds(3+receivetime), &MyReceiveFunction,Company->router.Get(0),20);//连路由接收
	Simulator::Schedule(Seconds(5.5+receivetime), &MySendFunction,Company->router.Get(0),50);//连路由发送
	Simulator::Schedule(Seconds(6.0+receivetime), &MyReceiveFunction,Battalion->router.Get(0),20);//营路由接收
	Simulator::Schedule(Seconds(9.2+receivetime), &MySendFunction,Battalion->router.Get(0),50);//营路由发送
	Simulator::Schedule(Seconds(9.7+receivetime), &MyReceiveFunction,Company->router.Get(0),20);//连路由接收
	
	Simulator::Schedule(Seconds(12.2+receivetime), &MySendFunction,Company->router.Get(0),50);//连路由发送
	Simulator::Schedule(Seconds(12.7+receivetime), &MyReceiveFunction,Platoon->drones.Get(0),20.0);//簇头接收
	Simulator::Schedule(Seconds(12.7+2*receivetime), &MySendFunction,Platoon->drones.Get(0),30.0);//簇头发送
	//终端接收
	Simulator::Schedule(Seconds(13.2+2*receivetime), &MyReceiveFunction,Platoon->phones.Get(0),20);
	double endtime=13.2+2*receivetime;
	//---------------------------------------------设置有线通信标志的移动-----------------------------------------
	//第一次连级通信标志
	uint32_t height=8;
	
	double companytime1=3.0+receivetime;
	
	/*
	Company->terminals.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-50,minposition-10,0.0));//智慧控制子网
	Company->terminals.Get(1)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-10,minposition+30,0.0));//情报收集子网
	Company->terminals.Get(2)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-50,minposition+30,0.0));//通信子网
	Company->switches.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-50,minposition+10,0.0));//交换机位置
	Company->router.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-50,minposition+50,0.0));//路由器位置
	*/
	netsimulyzer::NodeConfigurationHelper nodeConfigHelper2(orchestrator);

	//EnableMotionTrail：设置为 true，表示在可视化中启用节点的运动轨迹。
	nodeConfigHelper2.Set("EnableMotionTrail", BooleanValue(true));
	
	auto node1 = CreateObject<Node>();
	MobilityHelper mobility1;
	mobility1.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility1.Install(node1);
	nodeConfigHelper2.Set("Model",StringValue("models/transmission_sphere.obj"));
	nodeConfigHelper2.Set("Scale",ns3::DoubleValue(2.5));
	nodeConfigHelper2.Set("Name",ns3::StringValue(" "));
	nodeConfigHelper2.Set ("MotionTrailColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::YELLOW});
	nodeConfigHelper2.Install(node1);
	node1->GetObject<MobilityModel>()->SetPosition(Vector(minposition-50,minposition+50,-10.0));

	
	Simulator::Schedule(Seconds(companytime1+0.0), &UpdatePosition, node1, Vector3D(minposition-50, minposition+50, height)); //路由器接收消息
	Simulator::Schedule(Seconds(companytime1+0.1), &UpdatePosition, node1, Vector3D(minposition-50, minposition+45,height)); 
	Simulator::Schedule(Seconds(companytime1+0.2), &UpdatePosition, node1, Vector3D(minposition-50, minposition+40, height)); 
	Simulator::Schedule(Seconds(companytime1+0.3), &UpdatePosition, node1, Vector3D(minposition-50, minposition+35, height)); 
	Simulator::Schedule(Seconds(companytime1+0.4), &UpdatePosition, node1, Vector3D(minposition-50, minposition+30, height)); //到达通信子网
	Simulator::Schedule(Seconds(companytime1+0.5), &UpdatePosition, node1, Vector3D(minposition-50, minposition+25, height)); 
	Simulator::Schedule(Seconds(companytime1+0.6), &UpdatePosition, node1, Vector3D(minposition-50, minposition+20, height)); 
	Simulator::Schedule(Seconds(companytime1+0.7), &UpdatePosition, node1, Vector3D(minposition-50, minposition+15, height)); 
	Simulator::Schedule(Seconds(companytime1+0.8), &UpdatePosition, node1, Vector3D(minposition-50, minposition+10, height)); //到达交换机
	Simulator::Schedule(Seconds(companytime1+0.9), &UpdatePosition, node1, Vector3D(minposition-50, minposition+5, height)); 
	Simulator::Schedule(Seconds(companytime1+1.0), &UpdatePosition, node1, Vector3D(minposition-50, minposition+0, height)); 
	Simulator::Schedule(Seconds(companytime1+1.1), &UpdatePosition, node1, Vector3D(minposition-50, minposition-5, height)); 
	Simulator::Schedule(Seconds(companytime1+1.2), &UpdatePosition, node1, Vector3D(minposition-50, minposition-10, height)); //到达指挥控制子网
	//原路返回
	Simulator::Schedule(Seconds(companytime1+1.3), &UpdatePosition, node1, Vector3D(minposition-50, minposition-5, height)); 
	Simulator::Schedule(Seconds(companytime1+1.4), &UpdatePosition, node1, Vector3D(minposition-50, minposition+0, height)); 
	Simulator::Schedule(Seconds(companytime1+1.5), &UpdatePosition, node1, Vector3D(minposition-50, minposition+5, height)); 
	Simulator::Schedule(Seconds(companytime1+1.6), &UpdatePosition, node1, Vector3D(minposition-50, minposition+10, height)); //到达交换机
	Simulator::Schedule(Seconds(companytime1+1.7), &UpdatePosition, node1, Vector3D(minposition-50, minposition+15, height)); 
	Simulator::Schedule(Seconds(companytime1+1.8), &UpdatePosition, node1, Vector3D(minposition-50, minposition+20, height)); 
	Simulator::Schedule(Seconds(companytime1+1.9), &UpdatePosition, node1, Vector3D(minposition-50, minposition+25, height)); 
	Simulator::Schedule(Seconds(companytime1+2.0), &UpdatePosition, node1, Vector3D(minposition-50, minposition+30, height)); //到达通信子网
	Simulator::Schedule(Seconds(companytime1+2.1), &UpdatePosition, node1, Vector3D(minposition-50, minposition+35, height)); 
	Simulator::Schedule(Seconds(companytime1+2.2), &UpdatePosition, node1, Vector3D(minposition-50, minposition+40, height)); 
	Simulator::Schedule(Seconds(companytime1+2.3), &UpdatePosition, node1, Vector3D(minposition-50, minposition+45, height)); 
	Simulator::Schedule(Seconds(companytime1+2.4), &UpdatePosition, node1, Vector3D(minposition-50, minposition+50, height)); //到达路由器
	Simulator::Schedule(Seconds(companytime1+2.5), &UpdatePosition, node1, Vector3D(minposition-50, minposition+50, -10)); 
	
	for(double i = 0.01; i< 0.5;i += 0.01)
	{
		Simulator::Schedule(Seconds(companytime1+2.5+i), &UpdatePosition, node1, Vector3D(minposition-50, minposition+50, -10-500*i)); //删除指示器
	}
	//------------------------------------------------------------------------第二次连级通信标志---------------------------------------------
	double companytime2=9.7+receivetime;
	
	auto node2 = CreateObject<Node>();
	MobilityHelper mobility2;
	mobility2.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility2.Install(node2);
	nodeConfigHelper2.Set("Model",StringValue("models/transmission_sphere.obj"));
	nodeConfigHelper2.Set("Scale",ns3::DoubleValue(2.5));
	nodeConfigHelper2.Set("Name",ns3::StringValue(" "));
	nodeConfigHelper2.Set ("MotionTrailColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::YELLOW});
	nodeConfigHelper2.Install(node2);
	node2->GetObject<MobilityModel>()->SetPosition(Vector(minposition-50,minposition+50,-10.0));
	
	Simulator::Schedule(Seconds(companytime2+0.0), &UpdatePosition, node2, Vector3D(minposition-50, minposition+50, height)); //路由器接收消息
	Simulator::Schedule(Seconds(companytime2+0.1), &UpdatePosition, node2, Vector3D(minposition-50, minposition+45,height)); 
	Simulator::Schedule(Seconds(companytime2+0.2), &UpdatePosition, node2, Vector3D(minposition-50, minposition+40, height)); 
	Simulator::Schedule(Seconds(companytime2+0.3), &UpdatePosition, node2, Vector3D(minposition-50, minposition+35, height)); 
	Simulator::Schedule(Seconds(companytime2+0.4), &UpdatePosition, node2, Vector3D(minposition-50, minposition+30, height)); //到达通信子网
	Simulator::Schedule(Seconds(companytime2+0.5), &UpdatePosition, node2, Vector3D(minposition-50, minposition+25, height)); 
	Simulator::Schedule(Seconds(companytime2+0.6), &UpdatePosition, node2, Vector3D(minposition-50, minposition+20, height)); 
	Simulator::Schedule(Seconds(companytime2+0.7), &UpdatePosition, node2, Vector3D(minposition-50, minposition+15, height)); 
	Simulator::Schedule(Seconds(companytime2+0.8), &UpdatePosition, node2, Vector3D(minposition-50, minposition+10, height)); //到达交换机
	Simulator::Schedule(Seconds(companytime2+0.9), &UpdatePosition, node2, Vector3D(minposition-50, minposition+5, height)); 
	Simulator::Schedule(Seconds(companytime2+1.0), &UpdatePosition, node2, Vector3D(minposition-50, minposition+0, height)); 
	Simulator::Schedule(Seconds(companytime2+1.1), &UpdatePosition, node2, Vector3D(minposition-50, minposition-5, height)); 
	Simulator::Schedule(Seconds(companytime2+1.2), &UpdatePosition, node2, Vector3D(minposition-50, minposition-10, height)); //到达指挥控制子网
	//原路返回
	Simulator::Schedule(Seconds(companytime2+1.3), &UpdatePosition, node2, Vector3D(minposition-50, minposition-5, height)); 
	Simulator::Schedule(Seconds(companytime2+1.4), &UpdatePosition, node2, Vector3D(minposition-50, minposition+0, height)); 
	Simulator::Schedule(Seconds(companytime2+1.5), &UpdatePosition, node2, Vector3D(minposition-50, minposition+5, height)); 
	Simulator::Schedule(Seconds(companytime2+1.6), &UpdatePosition, node2, Vector3D(minposition-50, minposition+10, height)); //到达交换机
	Simulator::Schedule(Seconds(companytime2+1.7), &UpdatePosition, node2, Vector3D(minposition-50, minposition+15, height)); 
	Simulator::Schedule(Seconds(companytime2+1.8), &UpdatePosition, node2, Vector3D(minposition-50, minposition+20, height)); 
	Simulator::Schedule(Seconds(companytime2+1.9), &UpdatePosition, node2, Vector3D(minposition-50, minposition+25, height)); 
	Simulator::Schedule(Seconds(companytime2+2.0), &UpdatePosition, node2, Vector3D(minposition-50, minposition+30, height)); //到达通信子网
	Simulator::Schedule(Seconds(companytime2+2.1), &UpdatePosition, node2, Vector3D(minposition-50, minposition+35, height)); 
	Simulator::Schedule(Seconds(companytime2+2.2), &UpdatePosition, node2, Vector3D(minposition-50, minposition+40, height)); 
	Simulator::Schedule(Seconds(companytime2+2.3), &UpdatePosition, node2, Vector3D(minposition-50, minposition+45, height)); 
	Simulator::Schedule(Seconds(companytime2+2.4), &UpdatePosition, node2, Vector3D(minposition-50, minposition+50, height)); //到达路由器
	Simulator::Schedule(Seconds(companytime2+2.5), &UpdatePosition, node2, Vector3D(minposition-50, minposition+50, -10)); 
	
	for(double i = 0.01; i< 0.5;i += 0.01)
	{
		Simulator::Schedule(Seconds(endtime+i), &UpdatePosition, node2, Vector3D(minposition-50, minposition+50, -10-500*i)); //删除指示器
	}
	//-------------------------------------------------------------------------------营级通信标志--------------------------------------------------
	double battaliontime=6.0+receivetime;
	netsimulyzer::NodeConfigurationHelper nodeConfigHelper1(orchestrator);

	//EnableMotionTrail：设置为 true，表示在可视化中启用节点的运动轨迹。
	nodeConfigHelper1.Set("EnableMotionTrail", BooleanValue(true));
	
	auto node = CreateObject<Node>();
	MobilityHelper mobility0;
	mobility0.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility0.Install(node);
	nodeConfigHelper1.Set("Model",StringValue("models/transmission_sphere.obj"));
	nodeConfigHelper1.Set("Scale",ns3::DoubleValue(2.5));
	nodeConfigHelper1.Set("Name",ns3::StringValue(" "));
	nodeConfigHelper1.Set ("MotionTrailColor",netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::YELLOW});
	nodeConfigHelper1.Install(node);
	node->GetObject<MobilityModel>()->SetPosition(Vector(minposition-80,0,-10.0));
	// 动态更新位置
	/*
	Battalion->terminals.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-120, -40.0, 0.0)); // 节点坐标
	Battalion->terminals.Get(1)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-140, 0.0, 0.0)); // 节点坐标
	Battalion->terminals.Get(2)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-120, 40.0, 0.0)); // 节点坐标
	Battalion->terminals.Get(3)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-100, 0.0, 0.0)); // 节点坐标
	Battalion->switches.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-120, 0.0, 0.0)); // 交换机坐标
	Battalion->router.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(minposition-80, 0.0, 0.0)); // 路由器坐标坐标
	*/
	// 每隔0.1秒刷新一次位置
	Simulator::Schedule(Seconds(battaliontime), &UpdatePosition, node, Vector3D(minposition-80, 0, height)); //路由器接收消息
	Simulator::Schedule(Seconds(battaliontime+0.1), &UpdatePosition, node, Vector3D(minposition-85, 0,height)); 
	Simulator::Schedule(Seconds(battaliontime+0.2), &UpdatePosition, node, Vector3D(minposition-90, 0, height)); 
	Simulator::Schedule(Seconds(battaliontime+0.3), &UpdatePosition, node, Vector3D(minposition-95, 0, height)); 
	Simulator::Schedule(Seconds(battaliontime+0.4), &UpdatePosition, node, Vector3D(minposition-100, 0, height)); //到达通信子网
	Simulator::Schedule(Seconds(battaliontime+0.5), &UpdatePosition, node, Vector3D(minposition-105, 0, height)); 
	Simulator::Schedule(Seconds(battaliontime+0.6), &UpdatePosition, node, Vector3D(minposition-110, 0, height)); 
	Simulator::Schedule(Seconds(battaliontime+0.7), &UpdatePosition, node, Vector3D(minposition-115, 0, height)); 
	Simulator::Schedule(Seconds(battaliontime+0.8), &UpdatePosition, node, Vector3D(minposition-120, 0, height)); //到达交换机
	Simulator::Schedule(Seconds(battaliontime+0.9), &UpdatePosition, node, Vector3D(minposition-120, -5, height)); 
	Simulator::Schedule(Seconds(battaliontime+1.0), &UpdatePosition, node, Vector3D(minposition-120, -10, height)); 
	Simulator::Schedule(Seconds(battaliontime+1.1), &UpdatePosition, node, Vector3D(minposition-120, -15, height)); 
	Simulator::Schedule(Seconds(battaliontime+1.2), &UpdatePosition, node, Vector3D(minposition-120, -20, height)); 
	Simulator::Schedule(Seconds(battaliontime+1.3), &UpdatePosition, node, Vector3D(minposition-120, -25, height));
	Simulator::Schedule(Seconds(battaliontime+1.4), &UpdatePosition, node, Vector3D(minposition-120, -30, height));
	Simulator::Schedule(Seconds(battaliontime+1.5), &UpdatePosition, node, Vector3D(minposition-120, -35, height));
	Simulator::Schedule(Seconds(battaliontime+1.6), &UpdatePosition, node, Vector3D(minposition-120, -40, height));//到达指挥控制子网
	//原路返回
	Simulator::Schedule(Seconds(battaliontime+1.7), &UpdatePosition, node, Vector3D(minposition-120, -35, height));//离开指挥控制子网
	Simulator::Schedule(Seconds(battaliontime+1.8), &UpdatePosition, node, Vector3D(minposition-120, -30, height));
	Simulator::Schedule(Seconds(battaliontime+1.9), &UpdatePosition, node, Vector3D(minposition-120, -25, height));
	Simulator::Schedule(Seconds(battaliontime+2.0), &UpdatePosition, node, Vector3D(minposition-120, -20, height));
	Simulator::Schedule(Seconds(battaliontime+2.1), &UpdatePosition, node, Vector3D(minposition-120, -15, height));
	Simulator::Schedule(Seconds(battaliontime+2.2), &UpdatePosition, node, Vector3D(minposition-120, -10, height));
	Simulator::Schedule(Seconds(battaliontime+2.3), &UpdatePosition, node, Vector3D(minposition-120, -5, height));
	Simulator::Schedule(Seconds(battaliontime+2.4), &UpdatePosition, node, Vector3D(minposition-120, 0, height));//到达交换机
	Simulator::Schedule(Seconds(battaliontime+2.5), &UpdatePosition, node, Vector3D(minposition-115, 0, height));
	Simulator::Schedule(Seconds(battaliontime+2.6), &UpdatePosition, node, Vector3D(minposition-110, 0, height));
	Simulator::Schedule(Seconds(battaliontime+2.7), &UpdatePosition, node, Vector3D(minposition-105, 0, height));
	Simulator::Schedule(Seconds(battaliontime+2.8), &UpdatePosition, node, Vector3D(minposition-100, 0, height));//到达通信子网
	Simulator::Schedule(Seconds(battaliontime+2.9), &UpdatePosition, node, Vector3D(minposition-95, 0, height));
	Simulator::Schedule(Seconds(battaliontime+3.0), &UpdatePosition, node, Vector3D(minposition-90, 0, height));
	Simulator::Schedule(Seconds(battaliontime+3.1), &UpdatePosition, node, Vector3D(minposition-85, 0, height));
	Simulator::Schedule(Seconds(battaliontime+3.2), &UpdatePosition, node, Vector3D(minposition-80, 0, height));//到达路由器
	Simulator::Schedule(Seconds(battaliontime+3.3), &UpdatePosition, node, Vector3D(minposition-80, 0, -10.0));//隐藏指示模型
	
	for(double i = 0.01; i< 0.5;i += 0.01)
	{
		Simulator::Schedule(Seconds(endtime+i), &UpdatePosition, node, Vector3D(minposition-80, 0, -10-500*i)); //删除指示器
	}
	
	//------------------------------------------------------------无人机移动模型---------------------------------------------------------
	//以下无人机路径，flag为0时表明无人机在区域内执行正常任务
	//flag不为0时表明无人机出现异常行为
	if(flag == 0)
	{
		for(double i=0.1;i<1000;i += 0.1)
		{
			//前5秒让无人机从中心区域走到边界
			if(i<=5)
			{
				Simulator::Schedule(Seconds(i), &UpdatePosition, Platoon->uavs.Get(0), Vector3D(50+10*i, 170+5*i, 40));
			}
			//接下来让无人机在边界处做小幅度移动
			else
			{
				if(int(i/0.1)%4==0)Simulator::Schedule(Seconds(i), &UpdatePosition, Platoon->uavs.Get(0), Vector3D(100, 195, 40));
				else if (int(i/0.1)%4==1)Simulator::Schedule(Seconds(i), &UpdatePosition, Platoon->uavs.Get(0), Vector3D(100, 200, 40));
				else if (int(i/0.1)%4==2)Simulator::Schedule(Seconds(i), &UpdatePosition, Platoon->uavs.Get(0), Vector3D(100, 195, 40));
				else
				Simulator::Schedule(Seconds(i), &UpdatePosition, Platoon->uavs.Get(0), Vector3D(100, 190, 40));
			}
		}
	}
	else
	{
		
		for(double i=0.1;i<1000;i+=0.1)
		{
			//前5秒让无人机飞出去
			if(i<5)
			{
				Simulator::Schedule(Seconds(i), &UpdatePosition, Platoon->uavs.Get(0), Vector3D(100+5*i, 195+5*i, 40));//5秒后到达(125,220,40)
			}
			//接下来让无人机在某个区域内移动
			else
			{
				//做正方形运动
				int temp=int(i/0.1)%20;
				if(temp<5)
				Simulator::Schedule(Seconds(i), &UpdatePosition, Platoon->uavs.Get(0), Vector3D(125-10*temp, 220, 40));
				else 
				if(temp<10)Simulator::Schedule(Seconds(i), &UpdatePosition, Platoon->uavs.Get(0), Vector3D(120, 220-10*(temp-5), 40));
				else
				if(temp<15)Simulator::Schedule(Seconds(i), &UpdatePosition, Platoon->uavs.Get(0), Vector3D(120+10*(temp-10), 215, 40));
				else
				Simulator::Schedule(Seconds(i), &UpdatePosition, Platoon->uavs.Get(0), Vector3D(125, 215+10*(temp-15), 40));
			}
		}
		
	}
	
	
	AnimationInterface anim("XiDian_two_scene.xml");
	
	//设置仿真在指定时间结束
	Simulator::Stop(Seconds(duration));

	Simulator::Run();

	//在日志中记录仿真结束的消息
	*infoLog << "Scenario Finished\n";

	Simulator::Destroy();
	
}












































