#/bin/bash
lspci
dmesg -c

echo -e "\n Please Wait Loading Base Driver \n"
insmod nex_driver.ko debug_level=4 
sleep 5
dmesg -c

echo -e "	Application Menu : \n"

echo "			Enter 1  for MMC_EraseTrim"      
echo "			Enter 2  for MMC_PwdDevLock"
echo "			Enter 3  for SDMMC_BlockRW"
echo "			Enter 4  for SDMMC_SendCmdSingleBlkWrRd"
echo "			Enter 5  for SDMMC_SendCmdMultiBlkWrRd"
echo "			Enter 0  for EXIT"
echo -e "\n		Please Enter Your Choice \n"
read apps

if [ $apps -eq 1 ]
	then
	echo -e "033[34m Please enter number of Iterations \033[0m"
	read n
	for ((i=0 ;i<n ;i++))
		do
		echo -e "033[33m Iteration no.$i \033[0m"
	        echo -e "Loading MMC_EraseTrim"
		insmod nex_MMC_EraseTrim.ko
		sleep 5
		rmmod nex_MMC_EraseTrim.ko
		dmesg -c		
		done
		rmmod nex_driver.ko
		dmesg -c		
	
elif [ $apps -eq 2 ]
	then
	echo -e "033[34m Please enter number of Iterations \033[0m"
	read n
	for ((i=0 ;i<n ;i++))
		do
		echo -e "033[33m Iteration no.$i \033[0m"
	        echo -e "Loading MMC_PwdDevLock"
		insmod nex_MMC_PwdDevLock.ko
		sleep 5
		rmmod nex_MMC_PwdDevLock.ko
		dmesg -c		
		done
		rmmod nex_driver.ko
		dmesg -c
elif [ $apps -eq 3 ]
	then
	echo -e "033[34m Please enter number of Iterations \033[0m"
	read n
	for ((i=0 ;i<n ;i++))
		do
		echo -e "033[33m Iteration no.$i \033[0m"
	        echo -e "Loading SDMMC_BlockRW"
		insmod nex_SDMMC_BlockRW.ko
		sleep 5
		rmmod nex_SDMMC_BlockRW.ko
		dmesg -c		
		done
		rmmod nex_driver.ko
		dmesg -c
elif [ $apps -eq 4 ]
	then
	echo -e "033[34m Please enter number of Iterations \033[0m"
	read n
	for ((i=0 ;i<n ;i++))
		do
		echo -e "033[33m Iteration no.$i \033[0m"
	      	echo -e "Loading SDMMC_SendCmdSingleBlkWrRd"
		insmod nex_SDMMC_SendCmdMultiBlkWrRd.ko
		sleep 5
		rmmod nex_SDMMC_SendCmdMultiBlkWrRd.ko
		dmesg -c		
		done
		rmmod nex_driver.ko
		dmesg -c
elif [ $apps -eq 5 ]
	then
	echo -e "033[34m Please enter number of Iterations \033[0m"
	read n
	for ((i=0 ;i<n ;i++))
		do
		echo -e "033[33m Iteration no.$i \033[0m"
	        echo -e "Loading SDMMC_SendCmdMultiBlkWrRd"
		insmod nex_SDMMC_SendCmdSingleBlkWrRd.ko
		sleep 5		
		rmmod nex_SDMMC_SendCmdSingleBlkWrRd.ko
		dmesg -c
		done
		rmmod nex_driver.ko
		dmesg -c
elif [ $apps -eq 0 ]
	then
	echo -e "EXIT \n"
	rmmod nex_driver.ko
	dmesg -c
	exit 0
else
	rmmod nex_driver.ko
	dmesg -c
	echo -e "Please try again from the given option only !!!! \n"
	exit 0
fi
