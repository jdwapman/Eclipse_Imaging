#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Incorrect number of parameters"
	echo "Usage: ./eclipse_launch <numImages>"
	echo "Input -1 for continuous capture"
	exit -1
fi

cd /media/nvidia/SD_TX1/Output_Images/Camera_Images/

date=$(date +'%Y-%m-%d_%H-%M-%S')
mkdir ${date}
cd ${date}
mkdir Cam1 #Cam1, cam2 are subfolders within date folder
mkdir Cam2

#Instruct user to input calibrated colors.txt (Jonathan's note: not needed. File must be at ~/Eclipse/Target_Detection/colors.txt)

cd ~/Eclipse/Target_Detection/Release
make all

#    $1 contains number of images to capture. -1 for infinite.
./Target_Detection $1 /media/nvidia/SD_TX1/Output_Images/Camera_Images/${date}/Cam1 /media/nvidia/SD_TX1/Output_Images/Camera_Images/${date}/Cam2


#shutdown?
