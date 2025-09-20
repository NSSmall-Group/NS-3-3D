#!/bin/bash

while true
do
	if grep -q "0" data.txt;then
		echo "waiting..."
	else
		echo "under attack..."
		break
	fi
done

pid=$(pidof netsimulyzer)

if [[ -z "$pid" ]];then
        echo "can not find the process of netsimulyzer!"
else
        printf "pid equals $pid \n"
        kill -9 $pid
fi


sed -i '136c double flag=1;' main.cc
cd /home/user/Downloads/ns-allinone-3.38/ns-3.38
./ns3 run scratch/two_scene/main.cc
cd /home/user/Downloads/ns-allinone-3.38/ns-3.38/scratch/two_scene
sed -i '136c double flag=0;' main.cc
cd /home/user/Downloads/ns-allinone-3.38/ns-3.38
./NetSimulyzer/build/netsimulyzer XIDIAN_two_scene.json

#检测文件内容
#更改代码
#执行main函数
#用netsimulyzer打开json文件
