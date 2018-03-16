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

cd ~/Eclipse/Target_Detection/Debug
make all

folder1=/media/nvidia/SD_TX1/Output_Images/Camera_Images/${date}/Cam1
folder2=/media/nvidia/SD_TX1/Output_Images/Camera_Images/${date}/Cam2

#    $1 contains number of images to capture. -1 for infinite.
./Target_Detection $1 $folder1 #$folder2

#Stitch into video
cd $folder1
cat *.jpg | ffmpeg -f image2pipe -r 10 -vcodec mjpeg -i - -vcodec libx264 capture_$date.mp4

#Make Dropbox directory
~/Eclipse/Target_Detection/Dropbox-Uploader/dropbox_uploader.sh mkdir Eclipse_Telemetry/$date/Cam1

~/Eclipse/Target_Detection/Dropbox-Uploader/dropbox_uploader.sh mkdir Eclipse_Telemetry/$date/Cam2

#Upload
~/Eclipse/Target_Detection/Dropbox-Uploader/dropbox_uploader.sh upload * Eclipse_Telemetry/$date/Cam1
