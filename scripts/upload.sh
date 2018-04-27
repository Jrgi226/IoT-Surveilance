echo "Hey I am running upload!!"
echo "Current Date: $(date)" >> /home/iot-1/scripts/projectlog.log
cd /home/iot-1/build-UploadImage-Desktop_Qt_5_9_1_GCC_64bit-Debug
./UploadImage
printf "\n" >> /home/iot-1/scripts/projectlog.log

