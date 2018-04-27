echo "Hey I am at start!!"
echo "Current Date: $(date)" >> /home/iot-1/scripts/projectlog.log
echo This is where I am at: 
pwd
cd /dev
chmod 777 /ttyUSB0
printf "\n" >> /home/iot-1/scripts/projectlog.log

