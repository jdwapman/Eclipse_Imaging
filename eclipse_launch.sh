#!/bin/bash
cd ~/Desktop/
mkdir ITWORKED
cd /media/nvidia/SD_TX1/Output_Images/Camera_Images/

date=$(date +'%Y-%m-%d_%H-%M-%S')
name1="Cam1_${date}"
#name2="Cam2_${date}"
mkdir $name1
#mkdir $name2

#Instruct user to input calibrated colors.txt

cd ~/Eclipse/Target_Detection/Release
make all
./Target_Detection $name1 $name2


#shutdown?
