#!/bin/bash

cd /media/nvidia/SD_TX1/Output_Images/Camera_Images/

date=$(date +'%Y-%m-%d_%H-%M-%S')
mkdir ${date}
cd ${date}
mkdir Cam1
mkdir Cam2

#Instruct user to input calibrated colors.txt

cd ~/Eclipse/Target_Detection/Release
make all

./Target_Detection ${date}/Cam1 ${date}/Cam2


#shutdown?
