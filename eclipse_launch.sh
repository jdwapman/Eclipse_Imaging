#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Incorrect number of parameters"
	echo "Usage: ./eclipse_launch numImages numSources"
	echo "Input -1 for continuous capture"
	exit -1
fi

cd /media/nvidia/SD_TX1/Output_Images/Camera_Images/

date=$(date +'%Y-%m-%d_%H-%M-%S')
mkdir ${date}
cd ${date}
mkdir Cam1 #Cam1, cam2 are subfolders within date folder
mkdir Cam2

parent=/media/nvidia/SD_TX1/Output_Images/Camera_Images/${date}
folder1=/media/nvidia/SD_TX1/Output_Images/Camera_Images/${date}/Cam1
folder2=/media/nvidia/SD_TX1/Output_Images/Camera_Images/${date}/Cam2

cd ~/Eclipse/Target_Detection/Release
make all | tee $parent/compile_log_$date.txt

#    $1 contains number of images to capture. -1 for infinite.
if [ "$2" -eq 2 ]; then #2 cameras
	./Target_Detection $1 $folder1 $folder2 | tee $parent/capture_log_$date.txt
else
	./Target_Detection $1 $folder1 | tee $parent/capture_log_$date.txt
fi


#Stitch into videos
cd $folder1
cat *.jpg | ffmpeg -f image2pipe -r 10 -vcodec mjpeg -i - -vcodec libx264 capture_$date.mp4 | tee $parent/encode_cam1_log_$date.txt

cd $folder2
if [ "$2" -eq 2 ]; then
	cat *.jpg | ffmpeg -f image2pipe -r 10 -vcodec mjpeg -i - -vcodec libx264 capture_$date.mp4 | tee $parent/encode_cam2_log_$date.txt
fi

#Make Dropbox directory
~/Eclipse/Target_Detection/Dropbox-Uploader/dropbox_uploader.sh mkdir Eclipse_Telemetry/$date/Cam1

~/Eclipse/Target_Detection/Dropbox-Uploader/dropbox_uploader.sh mkdir Eclipse_Telemetry/$date/Cam2

#Upload
cd $folder1
~/Eclipse/Target_Detection/Dropbox-Uploader/dropbox_uploader.sh upload * Eclipse_Telemetry/$date/Cam1

cd $folder2
if [ "$2" -eq 2 ]; then
	~/Eclipse/Target_Detection/Dropbox-Uploader/dropbox_uploader.sh upload * Eclipse_Telemetry/$date/Cam2
fi

cd $parent
~/Eclipse/Target_Detection/Dropbox-Uploader/dropbox_uploader.sh upload *.txt Eclipse_Telemetry/$date/
