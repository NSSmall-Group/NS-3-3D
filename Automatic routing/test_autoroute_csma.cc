#include "ns3/ipv4-global-routing-helper.h"
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


using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("TestRoutingExample");

const char* outputFile = "output.txt";
const char* route_tree = "route_tree.txt";

void output(std::map<Ptr<Node>,Ptr<Channel>> neighbors,int i)
{
	// 创建一个ofstream对象，用于写入文件
	std::ofstream outFile;

	// 打开文件（如果文件不存在，会自动创建）
	outFile.open(outputFile, std::ios::app);

	// 检查文件是否成功打开
	if (!outFile.is_open()) std::cerr << "Failed to open file for writing!" << std::endl;

	// 写入内容
	outFile << "Node ID: "<< i <<  ":"  <<std::endl;
	for (auto& pair : neighbors) {
        Ptr<Node> neighbor = pair.first;
        Ptr<Channel> link = pair.second;
        outFile << " Neighbor Node ID: " << neighbor->GetId()
        	  //<< " Neighbor Node ID type: " <<typeid(neighbor->GetId()).name()//输出j表示为uint32_t类型
                  << " via Channel: " << link->GetId()
                  //<< " via Channel type: " <<typeid(link->GetId()).name()
                  << std::endl;
    }
    	outFile << " \n " << std::endl;
	// 关闭文件
	outFile.close();

	std::cout << "Content written to file successfully!" << std::endl;
}

// 获取指定节点的所有相邻节点及连接链路
std::map<Ptr<Node>, Ptr<Channel>> GetNeighborInfo(Ptr<Node> node,int i) {
    std::map<Ptr<Node>, Ptr<Channel>> neighbors;
    
    // 遍历节点的所有网络设备
    for (uint32_t devId = 0; devId < node->GetNDevices(); ++devId) {
        Ptr<NetDevice> localDev = node->GetDevice(devId);
        Ptr<Channel> channel = localDev->GetChannel();
        
        if (!channel) continue;  // 跳过无物理连接的设备
        
        // 获取信道上的所有设备
        for (uint32_t chDevId = 0; chDevId < channel->GetNDevices(); ++chDevId) {
            Ptr<NetDevice> remoteDev = channel->GetDevice(chDevId);
            
            // 排除自身设备
            if (remoteDev == localDev) continue;
            
            Ptr<Node> remoteNode = remoteDev->GetNode();
            neighbors[remoteNode] = channel;
        }
    }
    
    if(i>=0)output(neighbors,i);
    return neighbors;
}

// 使用示例
void ExampleUsage(Ptr<Node> currentNode,uint32_t i) {
    
    auto neighborMap = GetNeighborInfo(currentNode,i);
    
    for (auto& pair : neighborMap) {
        Ptr<Node> neighbor = pair.first;
        Ptr<Channel> link = pair.second;
        
        std::cout << " Neighbor Node ID: " << neighbor->GetId()
        	  << " Neighbor Node ID type: " <<typeid(neighbor->GetId()).name()//输出j表示为uint32_t类型
                  << " via Channel: " << link->GetId()
                  << " via Channel type: " <<typeid(link->GetId()).name()
                  << std::endl;
    }
}

void Get_Map(void)
{
	if(std::remove(outputFile) == 0)
	{
		std::cout << "File " << outputFile << " deleted successfully." << std::endl;
	}
	else
	{
		std::cerr << "Error deleting file " << outputFile << std::endl;
	}
	for(uint32_t i=0;i<NodeList::GetNNodes();i++)
	{
		Ptr<Node>node = NodeList::GetNode(i);//根据序号获取参数
		ExampleUsage(node,i);
	}
}

typedef struct node
{
	uint32_t nodeid;//节点序号
	int linkid;//链路序号
	struct node * first;//指向第一个孩子节点
	struct node * last;//指向最后一个孩子节点
	struct node * fnode;//指向父节点
	struct node * next;//指向下一个兄弟节点
	int terminal_nodeid;//初始值为负值，表示不为终止节点，当该值不为负时直接转换为uint32_t类型即可
}node;

std::queue<node *> q;//队列

//nodeid表示当前node的id序号，fnode表示父节点，linkid表示从父节点过来的链路
node * initialize_node(uint32_t nodeid,node * fnode,int linkid)
{
	node * temp = new node();
	temp->nodeid = nodeid;//节点id
	temp->linkid=linkid;//链路id
	temp->first = nullptr;
	temp->last = nullptr;
	temp->next=nullptr;
	temp->fnode=fnode;
	temp->terminal_nodeid=-1;//-1表示不为终止节点
	return temp;
}

//判断节点是否是终止节点，若是，则用terminal_nodeid记录重复节点序号，其中u表示当前搜索的序号，也表示向上搜索的终止节点
bool judge_terminal(node *temp,uint32_t u)
{
	if(temp == nullptr)return false;
	//if(temp->nodeid == u)return true;//这里判断的太简单了，导致根节点也会被判断为终止节点，应当当节点向上移动一步后再判断
	uint32_t i = temp->nodeid;
	temp=temp->fnode;
	while(temp != nullptr)//
	{
		if(temp->nodeid == i)return true;//先判断是否重复后判断是否到达搜索节点
		if(temp->nodeid == u && temp->terminal_nodeid == -1)break;//表示到达搜索节点，搜索节点的id为u且不为某一节点的终止节点
		temp=temp->fnode;
	}
	return false;
}

//给定父节点，链路名称，与孩子节点名称，创建边与孩子节点链接
void add_link_node(node *temp,uint32_t linkid,uint32_t nodeid)
{
	if(temp == nullptr)return;
	node *cnode=initialize_node(nodeid,temp,int(linkid));
	if(temp->first==nullptr)
	{
		temp->first=temp->last=cnode;
	}
	else
	{
		temp->last->next=cnode;
		temp->last=cnode;
	}
}

//检查节点的孩子节点是否包含i
bool check_exist(node *temp,uint32_t i)
{
	if(temp==nullptr)return false;
	node *flag=temp->first;
	while(flag != nullptr)
	{
		if(flag->nodeid == i)return true;
		flag=flag->next;
	}
	return false;
}

//给定节点，将其能到达的全部节点和边加入路由树
void add_node(node *temp)
{
	Ptr<Node> currentnode= NodeList::GetNode(temp->nodeid);//根据序号获取参数
	auto neighborMap = GetNeighborInfo(currentnode,-1);//获取邻居参数而不输出内容到outputFile文件中
    
	for (auto& pair : neighborMap) {
		Ptr<Node> neighbor = pair.first;
		Ptr<Channel> link = pair.second;
	  	//针对每一个到达的节点，都检查是否已在当前节点的孩子节点中存在，若存在则跳过添加，若不存在则添加该节点
	  	uint32_t i=neighbor->GetId();//获取邻居序号
	  	if(! check_exist(temp,i))//判断i是否已存在于temp的孩子节点中,若不存在，则添加该节点
	  	{
	  		//std::cout<<"节点"<<temp->nodeid<<"已添加孩子节点"<<i<<std::endl;
	  		add_link_node(temp,link->GetId(),i);
	  		}
	}
}

//给定节点，判断当前节点是否是终止节点，若不是，则“更新”全部能到达的边和节点
void update_child_node(node *temp,uint32_t i)
{
	if(temp == nullptr)return;
	if(judge_terminal(temp,i) == true)//是终止节点
	{
		temp->terminal_nodeid=int(i);
		}
	else//不是终止节点
		add_node(temp);//更新路由树
}

bool judge_first(bool label[],int num,uint32_t i)
{
	if(int(i)>=num || int(i)<0)return true;
	else
		return label[int(i)];
}

void add_queue(node * temp)//将temp的全部孩子节点入队
{
	if(temp == nullptr || temp->first == nullptr)return;
	node *flag=temp->first;
	while(flag!=nullptr)
	{
		q.push(flag);
		flag=flag->next;
	}
}

//遍历树
void display_tree(node *root)
{
	if(root==nullptr)return;
	node *flag=root->first;
	while(flag!=nullptr)
	{
		display_tree(flag);
		flag=flag->next;
	}
	std::cout<< root->nodeid << std::endl;
}

//获取树节点数量
void tree_num(node *root,int *number)
{
	if(root==nullptr||number==nullptr)return;
	node *flag=root->first;
	while(flag!=nullptr)
	{
		tree_num(flag,number);
		flag=flag->next;
	}
	(*number)++;
}

//判断节点a是否是节点b的父节点
//若节点a是节点b的父节点，那么节点b的终止节点也一定是节点a的终止节点
bool judge_parent(node *a,node *b)
{
	if(a == nullptr || b ==nullptr) return false;
	b=b->fnode;
	while(b && b != a )b=b->fnode;
	if(b != nullptr) return true;
	return false;
}

//搜索路由节点a的每一颗子树判断是否存在c，从而找到下一跳节点b
//找不到返回-1
node* search(node *root,uint32_t c,int nodeid,node * nodecontainer[],int num)
{
	q=std::queue<node*>();//清空队列
	q.push(root);
	while(!q.empty())
	{
		node *temp=q.front();
		q.pop();
		if(temp->nodeid == c) //找到c了
		{
			while(temp!= nullptr && temp->fnode != root)temp=temp->fnode;
			return temp;
		}
		node *flag=temp->first;
		while(flag != nullptr)
		{
			if(flag->terminal_nodeid >=num)
			{
				std::cout<<"路由树构建有误！！！"<<std::endl;
				return nullptr;
			}
			if(flag->terminal_nodeid != nodeid || (flag->terminal_nodeid >=0 && judge_parent(root,nodecontainer[flag->terminal_nodeid])==false))//非终止节点，入队
				q.push(flag);
			flag=flag->next;
		}	
	}
	return nullptr;
}

//根据节点序号和链路序号获取接口索引
int get_order_of_link(uint32_t nodeid,uint32_t linkid)
{
	Ptr<Node>target = NodeList::GetNode(nodeid);
	for (uint32_t devId=0; devId < target->GetNDevices(); ++devId) {
		Ptr<NetDevice> localDev = target->GetDevice(uint32_t(devId));
		Ptr<Channel> channel = localDev->GetChannel();
		if (!channel) continue;  // 跳过无物理连接的设备
		if(channel->GetId() == linkid)return int(devId)+1;//此时devId+1即为出口索引
	}
	return -1;
}

Ipv4Address get_Address(uint32_t nodeid,uint32_t linkid)
{
	Ptr<Node>target = NodeList::GetNode(nodeid);
	Ptr<Ipv4> ipv4 = target->GetObject<Ipv4>();//获取目标节点的Ipv4对象
	int targetid=get_order_of_link(nodeid,linkid);//获取nodeid中linkid的出口索引
	if(targetid == -1)
	{
		std::cout<<"2:路由树构建有误或网络设备未分配IP地址！！无法找到接口索引！！"<<std::endl;
		return Ipv4Address();//空地址
	}
	Ipv4Address ipAddress = ipv4->GetAddress(targetid, 0).GetLocal();//获取下一跳节点的IP地址
	return ipAddress;
}

//根据路由树配置全部节点的静态路由
void configure_static_route(node *root,node *nodecontainer[],int num)//三个参数分别表示路由树、保存的第一次出现的节点、节点数量，这里其实用不到root了
{
	//int num =NodeList::GetNNodes();//获取节点数量
	for(uint32_t i=0;int(i)<num;i++)//每一轮循环配置节点i的静态路由
	{
		if(nodecontainer[i]==nullptr)
		{
			std::cout<<"nodecontainer有误!"<<std::endl;
			continue;
		}
		Ptr<Node>target = NodeList::GetNode(i);//根据节点序号i获得节点
		for(uint32_t j=0;int(j)<num;j++)
		{
			if(i == j)continue;//不配置到达自己的静态路由
			//判断i到达j走哪个链路（假设为I），以及下一跳节点是什么（假设为b）
			//逐个判断各个子树的根节点是否包含j
			node *temp=search(nodecontainer[i],j,i,nodecontainer,num);
			if(temp==nullptr)continue;//i无法到达j
			uint32_t nodeid=temp->nodeid;//到达j的下一跳节点序号
			uint32_t linkid=temp->linkid;//到达j的下一跳链路
			//判断I在a的出口索引是什么
			int order=get_order_of_link(i,linkid);
			if(order == -1)
			{
				std::cout<<"1:路由树构建有误或网络设备未分配IP地址！！无法找到接口索引！！"<<std::endl;
				return;
			}
			//下一跳节点b在链路I的地址是什么
			Ipv4Address ipAddress=get_Address(nodeid,linkid);//获取链路linkid上nodeid的ip地址
			//std::cout<<"节点"<<nodeid<<"在链路"<<linkid<<"上的IP地址为:"<<ipAddress<<std::endl;
			if(ipAddress == Ipv4Address())
			{
				std::cout<<"地址获取失败！"<<std::endl;
				continue;
			}
			//获取目标节点的子网IP与子网掩码(应该考虑目标节点的全部网络设备，每一个网络设备表示该节点具有的一个IP，)
			//获取每一个网络设备的子网IP与子网掩码
			Ptr<Node>temp1=NodeList::GetNode(j);//获取目标节点,下一步是为目标节点的每一个网络设备配置一条静态路由
			Ptr<Ipv4> ipv4 = temp1->GetObject<Ipv4>();
			 // 遍历节点的所有网络设备
		        for (uint32_t devId = 0; devId < temp1->GetNDevices(); ++devId) {
				Ptr<NetDevice> device = temp1->GetDevice(devId);
			        int32_t interfaceIndex = ipv4->GetInterfaceForDevice(device);
			        if (interfaceIndex < 0) continue; // 跳过无效接口
			        
				Ipv4Address ip = ipv4->GetAddress(devId, 0).GetLocal();//获取第devId个设备的IP
				if(ip == Ipv4Address("127.0.0.1"))continue;
				//Ipv4Mask mask = ipv4->GetAddress(devId, 0).GetMask();//获取第devId个设备的子网掩码
				//Ipv4Address subnet = ip.CombineMask(mask);//获取第devId个设备的子网地址
				//做一个判断，当下一跳地址与目标地址不同时，再配置静态路由
				//如果下一跳地址与目标地址相同，说明此时是两个相邻节点（处在同一链路中），不需要配置静态路由
				if(ipAddress != ip)
				{
					//配置静态路由
					Ptr<Ipv4StaticRouting> staticRoutingNode = Ipv4RoutingHelper::GetRouting<Ipv4StaticRouting>(
					    target->GetObject<Ipv4>()->GetRoutingProtocol()
					    );
					std::cout<<"由节点"<<i<<"到达节点"<<j<<"的下一跳为"<<nodeid<<"下一跳IP地址为"<<ipAddress<<"出口索引为"<<order<<"目标设备IP地址为"<<ip<<std::endl;
					staticRoutingNode->AddNetworkRouteTo(
					    	//subnet,
					    	//mask,
					    	ip,
					    	"255.255.255.255",
					    	ipAddress,
					    	uint32_t(order)
					 );
				 }
		        }
		}
	}
}

// 递归打印树形结构
void printTreeInternal(node* currentNode, const std::string& prefix, bool isLast) {
    if (!currentNode) return;
 
    // 打印当前节点信息
    std::cout << prefix;
    std::cout << (isLast ? "└── " : "├── ");
    std::cout << "Node " << currentNode->nodeid
              << " (Link " << currentNode->linkid
              << ", Parent: " << (currentNode->fnode ? std::to_string(currentNode->fnode->nodeid) : "None")
              << (currentNode->terminal_nodeid >= 0 ? ", Terminal: "+std::to_string(currentNode->terminal_nodeid) : "")
              << ")" << std::endl;
 
    // 递归处理子节点
    node* child = currentNode->first;
    while (child) {
        bool lastChild = (child->next == nullptr);
        std::string newPrefix = prefix + (isLast ? "    " : "│   ");
        printTreeInternal(child, newPrefix, lastChild);
        child = child->next;
    }
}

// 入口函数
void printTree(node* root) {
    if (!root) {
        std::cout << "Tree is empty!" << std::endl;
        return;
    }
    std::cout << "Tree Structure:" << std::endl;
    printTreeInternal(root, "", true); // 根节点视为最后一个节点，无需兄弟缩进
}

//构建路由树并输出到route_tree.txt文件中
void construct_route_tree()
{
	int num=NodeList::GetNNodes();
	if(num == 0)return;
	bool label[num];//标签，用于记录第i个节点是否在路由树中出现过
	node * nodecontainer[num];//创建N个node指针，分别指向第一次出现的节点
	uint32_t order[num];//用于记录每一类节点出现的第一个节点的出现次序，并根据此次序更新路由树
	for(int i=0;i<num;i++)
	{
		label[i]=false;
		order[i]=uint32_t(-1);
		nodecontainer[i]=nullptr;
		}
	label[0]=true;
	int flag=0,flag1=1;//flag用来指向当前进度，flag1指向order内容,flag1初始化为1
	order[0]=0;
	
	node *root=initialize_node(0,nullptr,-1);//从0开始，无父节点，无链路
	nodecontainer[0]=root;//root为0号节点第一次出现
	q.push(root);
	//判断队列是否为空，不为空，则pop一个节点，加入其全部子节点(只有当其为终止节点时，才取消加入)
	while(flag<flag1&&flag1<=num)
	{
		while(!q.empty())//检查队列是否为空
		{
			//pop一次队列则在队列和路由树中分别更新数据；再检查当前节点是否是节点第一次出现,若是，则标记。
			node *temp=q.front();//获取队首元素
			q.pop();//弹出队首元素
			update_child_node(temp,order[flag]);//首先判断节点是否是终止节点，若是则不更新，否则更新其全部孩子节点
			add_queue(temp);//根据上一步更新后的路由树更新队列
			//判断该元素是否是第一次出现
			if(!judge_first(label,num,temp->nodeid))
			{
				nodecontainer[temp->nodeid]=temp;//标记节点
				label[temp->nodeid]=true;
				order[flag1]=temp->nodeid;
				flag1++;
				}
			//std::cout<<"队列的元素个数为:"<<q.size()<<std::endl;
		}
		//若队列为空，则同时flag++,找到下一个找到的某一类的第一个节点，让其入队
		flag++;
		//找到下一个节点，让其入队
		if(flag<flag1&&flag1<=num)
		{
			if(int(order[flag])<num&& int(order[flag])>=0 &&nodecontainer[int(order[flag])]!=nullptr)
				q.push(nodecontainer[int(order[flag])]);
			else
			{
				if(int(order[flag])<num)printf("order[flag]越界");
				else
				printf("nodecontainer[order[flag]] == nullptr");
			}
		}
	}
	int number=0;
	//display_tree(root);
	printTree(root);
	tree_num(root,&number);
	std::cout<<"\n路由树的节点数量为"<<number<<std::endl;
	configure_static_route(root,nodecontainer,num);
	printf("\n");
}

int main(int argc, char *argv[]) 
{
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // 创建节点
    NodeContainer global;
    global.Create(11); // 使用Create方法更简洁地创建8个节点
    Ptr<Node> n0 = global.Get(0);
    Ptr<Node> n1 = global.Get(1);
    Ptr<Node> n2 = global.Get(2);
    Ptr<Node> n3 = global.Get(3);
    Ptr<Node> n4 = global.Get(4);
    Ptr<Node> n5 = global.Get(5);
    Ptr<Node> n6 = global.Get(6);
    Ptr<Node> n7 = global.Get(7);
    Ptr<Node> n8 = global.Get(8);
    Ptr<Node> n9 = global.Get(9);
    Ptr<Node> n10 = global.Get(10);
   

    //设置有线助手
CsmaHelper csma;
//设置数据传输率
csma.SetChannelAttribute("DataRate",StringValue("100Mbps"));
//设置时延为6560纳秒，即6.56微秒
csma.SetChannelAttribute("Delay",TimeValue(NanoSeconds(6560)));
NetDeviceContainer csmadevice=csma.Install(NodeContainer(n5,n6,n7));

    // 配置点对点链路
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer d0d1 = p2p.Install(NodeContainer(n0, n1));
    NetDeviceContainer d1d2 = p2p.Install(NodeContainer(n1, n2));
    NetDeviceContainer d1d3 = p2p.Install(NodeContainer(n1, n3));
    NetDeviceContainer d2d4 = p2p.Install(NodeContainer(n2, n4));
    NetDeviceContainer d3d4 = p2p.Install(NodeContainer(n3, n4));
    NetDeviceContainer d4d5 = p2p.Install(NodeContainer(n4, n5));
    NetDeviceContainer d8d1 = p2p.Install(NodeContainer(n8, n1));
    NetDeviceContainer d9d1 = p2p.Install(NodeContainer(n9, n1));
    NetDeviceContainer d10d1 = p2p.Install(NodeContainer(n10, n1));

    // 配置IP地址
    InternetStackHelper internet;
    internet.Install(global);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer i0i1 = ipv4.Assign(d0d1);

    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i1i2 = ipv4.Assign(d1d2);

    ipv4.SetBase("10.2.2.0", "255.255.255.0");
    Ipv4InterfaceContainer i1i3 = ipv4.Assign(d1d3);

    ipv4.SetBase("10.3.3.0", "255.255.255.0");
    Ipv4InterfaceContainer i2i4 = ipv4.Assign(d2d4);

    ipv4.SetBase("10.4.4.0", "255.255.255.0");
    Ipv4InterfaceContainer i3i4 = ipv4.Assign(d3d4);

    // 正确分配子网地址
    ipv4.SetBase("10.6.6.0", "255.255.255.0");
    Ipv4InterfaceContainer i5i6i7 = ipv4.Assign(csmadevice);
    
    ipv4.SetBase("10.5.5.0", "255.255.255.0");
    Ipv4InterfaceContainer i4i5 = ipv4.Assign(d4d5);
    
    ipv4.SetBase("10.7.7.0", "255.255.255.0");
    Ipv4InterfaceContainer i8i1 = ipv4.Assign(d8d1);
    
    ipv4.SetBase("10.8.8.0", "255.255.255.0");
    Ipv4InterfaceContainer i9i1 = ipv4.Assign(d9d1);
    
    ipv4.SetBase("10.9.9.0", "255.255.255.0");
    Ipv4InterfaceContainer i10i1 = ipv4.Assign(d10d1);
	
	construct_route_tree();
	
    // 配置应用
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(n7);
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    // 获取n6的WiFi接口地址（索引1，因为apDevices是第一个设备）
    //----------创建多个输出设备
    UdpEchoClientHelper echoClient(i5i6i7.GetAddress(2), 9); 
    echoClient.SetAttribute("MaxPackets", UintegerValue(10));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps = echoClient.Install(n3);
    clientApps.Start(Seconds(2.1));
    clientApps.Stop(Seconds(10.0));
    
    UdpEchoClientHelper echoClient2(i5i6i7.GetAddress(2), 9); 
    echoClient2.SetAttribute("MaxPackets", UintegerValue(10));
    echoClient2.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient2.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps2 = echoClient2.Install(n2);
    clientApps2.Start(Seconds(2.7));
    clientApps2.Stop(Seconds(10.0));
    
    UdpEchoClientHelper echoClient3(i5i6i7.GetAddress(2), 9); 
    echoClient3.SetAttribute("MaxPackets", UintegerValue(10));
    echoClient3.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient3.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps3 = echoClient3.Install(n1);
    clientApps3.Start(Seconds(2.2));
    clientApps3.Stop(Seconds(10.0));
    
    UdpEchoClientHelper echoClient1(i5i6i7.GetAddress(2), 9); 
    echoClient1.SetAttribute("MaxPackets", UintegerValue(10));
    echoClient1.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient1.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps1 = echoClient1.Install(n0);
    clientApps1.Start(Seconds(2.0));
    clientApps1.Stop(Seconds(10.0));
    
    UdpEchoClientHelper echoClient4(i5i6i7.GetAddress(2), 9); 
    echoClient4.SetAttribute("MaxPackets", UintegerValue(10));
    echoClient4.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient4.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps4 = echoClient4.Install(n8);
    clientApps4.Start(Seconds(2.0));
    clientApps4.Stop(Seconds(10.0));
    
    UdpEchoClientHelper echoClient5(i5i6i7.GetAddress(2), 9); 
    echoClient5.SetAttribute("MaxPackets", UintegerValue(10));
    echoClient5.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient5.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps5 = echoClient5.Install(n9);
    clientApps5.Start(Seconds(2.0));
    clientApps5.Stop(Seconds(10.0));
    
    UdpEchoClientHelper echoClient6(i5i6i7.GetAddress(2), 9); 
    echoClient6.SetAttribute("MaxPackets", UintegerValue(10));
    echoClient6.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient6.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps6 = echoClient6.Install(n10);
    clientApps6.Start(Seconds(2.0));
    clientApps6.Stop(Seconds(10.0));
    
    // 配置移动模型
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(global);
	/*
	// 创建一个 OutputStreamWrapper，将 std::cout 包装为 Ptr<OutputStreamWrapper>
Ptr<OutputStreamWrapper> stream = Create<OutputStreamWrapper>(&std::cout);
//在仿真时间1.0秒时打印所有节点的路由表
Ipv4GlobalRoutingHelper::PrintRoutingTableAllAt(Seconds(1.0), stream);
	*/
    // 启用NetAnim
    AnimationInterface anim("animation.xml");
    anim.SetConstantPosition(n0, 0.0, 50.0);
    anim.SetConstantPosition(n1, 20.0, 50.0);
    anim.SetConstantPosition(n2, 40.0, 30.0);
    anim.SetConstantPosition(n3, 40.0, 70.0);
    anim.SetConstantPosition(n4, 60.0, 50.0);
    anim.SetConstantPosition(n5, 80.0, 50.0);
    anim.SetConstantPosition(n6, 80.0, 30.0);
    anim.SetConstantPosition(n7, 80.0, 70.0);
    anim.SetConstantPosition(n8, 0.0, 20.0);
    anim.SetConstantPosition(n9, 0.0, 30.0);
    anim.SetConstantPosition(n10, 0.0, 70.0);

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
