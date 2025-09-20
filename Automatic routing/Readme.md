NS-3中的全局路由寻址策略不允许网络中有环路存在，在某些特定需求下可能需要网络中存在环路
因此，我在NS-3中设计了一种寻址策略
思路：
  1. 任意选择节点A，按照广度优先遍历方法构建路由树，在该树中，若任意节点存在一个相同的祖先节点，则不再发展子树，如下图所示。
     <img width="1280" height="746" alt="image" src="https://github.com/user-attachments/assets/60278868-08d2-4cbe-8da3-2d4672107ddc" />
  2. 在该树中自顶向下按层寻找第一次出现的节点，将其标红，记录当前位置，并在该树上以同样的方法向下构树，直到树中全部节点不再是第一次出现的节点
    <img width="1280" height="733" alt="image" src="https://github.com/user-attachments/assets/a81edc0b-f888-4e53-8e26-9e4844387bd0" />
  3. 在该树上若想找到任意路径（如B到达F的全部路径），只需找到B第一次出现的位置，在其子树下找到全部F即可（除终止节点之外的全部F），在ns3应用中可根据此图快速找到路由（子树中层级最高的目标节点的路径即为最优路径）
    <img width="1280" height="761" alt="image" src="https://github.com/user-attachments/assets/5ce95894-8528-403b-beaa-a90d4b4b215a" />
   4. 链路阻塞之后的路由更新？链路阻塞后可在图中删除对应连接即可，例如当BC阻塞时，可以得到图为
    <img width="1280" height="775" alt="image" src="https://github.com/user-attachments/assets/04ca10d0-6d9a-4efc-800a-de2d8c01a2ca" />

优点：在ns3固定拓扑中，构树过程只需要一次，时间空间复杂度都较低，且可以应用于有环拓扑
缺点：在节点动态变化较为频繁的场景下，不适合用该方法（对于节点的删除较为简单，对于新增节点较为复杂）

实验：

在csma链路中实验，如下所示：在同时不配置静态路由、具备环路和csma信道的网络拓扑中，将udp客户机和服务器安装到同一csma信道中，启动仿真发现可以正常通信
  <img width="1280" height="754" alt="image" src="https://github.com/user-attachments/assets/11898eff-0f4f-4bd4-86b5-2be56b61076a" />
