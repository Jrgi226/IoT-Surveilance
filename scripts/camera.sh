echo "Hey I am running camera!!"
echo "Current Date: $(date)" >> /home/iot-1/scripts/projectlog.log
cd /home/iot-1/build-LAURecordRawVideo-Desktop_Qt_5_9_1_GCC_64bit-Debug
./LAURealSense /root/home/iot-1/TestImages 00:01:00
printf "\n" >> /home/iot-1/scripts/projectlog.log

