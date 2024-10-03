#/bin/bash
>/var/log/messages


#################################FUNCTION DEFINITION ##################################
data_transfer_sdhc0()
{
echo -e "** Doing Data Transfer **\n"
if [ -d /mnt/card ] 
        then 
	        echo "card dir already exists"
        else
	        mkdir /mnt/card
        fi


if [ -d /home/card_read ] 
        then 
	        echo "card_read dir already exists"
        else
	        mkdir /home/card_read
        fi


echo "Mounting the card BlockDev0p1...\n";
mount -t ext2 /dev/BlockDev0p1 /mnt/card/
sleep 5;

echo "\nCopy the files to be transferred to the card in /apps folder of the stack\n"
echo "Enter the filename to be written to the card...\n";
read FILENAME1 

WR_PATH1="/mnt/card/$FILENAME1"
RD_PATH1="/home/card_read/$FILENAME1"

echo -e "\n Writing $FILENAME1 to Card:\n"
cp $FILENAME1 $WR_PATH1 
sync
sleep 10


echo -e "\n Reading $FILENAME1 from Card:\n"
cp $WR_PATH1 $RD_PATH1 
sync
sleep 10


echo -e "\n Comparing file read from Card - ($RDPATH1) with Original file - (/apps/$FILENAME1) :\n"
cmp $FILENAME1 $RD_PATH1
status=$?
if [ $status -eq 0 ]
then
        currenttime=`date +%Hh_%Mm_%Ss`
        echo "$currenttime: COMPARED READ AND WRITE; FILE SIZE ARE EQUIVALENT"
        echo "$currenttime: COMPARED READ AND WRITE; FILE SIZE ARE EQUIVALENT"
else
        currenttime=`date +%Hh_%Mm_%Ss`
        echo "$currenttime: COMPARED READ AND WRITE; FILE SIZE ARE NOT EQUIVALENT"
        echo "$currenttime: COMPARED READ AND WRITE; FILE SIZE ARE NOT EQUIVALENT" 
fi


echo -e "Deleting the files
	$WR_PATH1 \n
	$RD_PATH1 \n"

rm -f $WR_PATH1
rm -f $RD_PATH1

echo -e "\n Unmounting card \n"
umount /dev/BlockDev0p1

echo -e "** Completed Data Transfer **\n"

}



sd_fs25_mode_1bit_sdhc0()
{    
	echo -e "\n*** Loading SD FullSpeed - 1bit module ***\n"
	insmod nex_SD_FS25MHz_1bit.ko sdhc=0
	sleep 2
	#echo -e "Unloading SD FullSpeed module...\n"
	rmmod nex_SD_FS25MHz_1bit.ko
}


sd_fs25_mode_4bit_sdhc0()
{    
	echo -e "\n*** Loading SD FullSpeed - 4bit module ***\n"
	insmod nex_SD_FS25MHz_4bit.ko sdhc=0
	sleep 2
	#echo -e "Unloading SD FullSpeed module...\n"
	rmmod nex_SD_FS25MHz_4bit.ko
}



sd_hs50_mode_1bit_sdhc0()
{    
	echo -e "\n*** Loading SD HighSpeed - 1bit module ***\n"
	insmod nex_SD_HS50MHz_1bit.ko sdhc=0
	sleep 2
	#echo -e "Unloading SD HighSpeed module...\n"
	rmmod nex_SD_HS50MHz_1bit.ko
}



sd_hs50_mode_4bit_sdhc0()
{    
	echo -e "\n*** Loading SD HighSpeed - 4bit module ***\n"
	insmod nex_SD_HS50MHz_4bit.ko sdhc=0
	sleep 2
	#echo -e "Unloading SD HighSpeed module...\n"
	rmmod nex_SD_HS50MHz_4bit.ko 
}




SD_2_0_CARD_SDHC0()
{
	echo -e "Select the Frequency modes for the SD2.0/eSD card:\n
	0 => Fullspeed \n
	1 => HighSpeed \n"
echo -e "??"
	read freqMode1;
	case $freqMode1 in
	
		0) 
			echo -e "Select the Buswidth:\n
	1 => 1bit\n
	4 => 4bit\n"
echo -e "??"
			read buswidth1
			case $buswidth1 in
			
				1)
					echo "SD Fullspeed mode has been selected with the Buswidth as 1-bit\n"
					sd_fs25_mode_1bit_sdhc0
				;;
				4)
					echo "SD Fullspeed mode has been selected with the Buswidth as 4-bit\n"
					sd_fs25_mode_4bit_sdhc0
				;;
			esac
		;;
	
		1)
			echo -e "Select the Buswidth:\n
	1 => 1bit\n
	4 => 4bit\n:"
echo -e "??"
			read buswidth2
			case $buswidth2 in
			
				1)
					echo -e "SD HighSpeed mode has been selected with the Buswidth as 1-bit\n"
					sd_hs50_mode_1bit_sdhc0
				;;
				4)
					echo -e "SD HighSpeed mode has been selected with the Buswidth as 4-bit\n"
					sd_hs50_mode_4bit_sdhc0
				;;
			esac
		;;
	esac
}



eSD_CARD_SDHC0()
{
	SD_2_0_CARD_SDHC0
}



sdr12_mode_sdhc0()
{    
	echo -e "\n*** Loading SDR12 - 4bit module ***\n"
	insmod nex_SD_SDR12.ko  sdhc=0
	sleep 2
	#echo -e "Unloading SDR12 module...\n"
	rmmod nex_SD_SDR12.ko 
}



sdr25_mode_sdhc0()
{    
	echo -e "\n*** Loading SDR25 - 4bit module ***\n"
	insmod nex_SD_SDR25.ko sdhc=0
	sleep 2
	#echo -e "Unloading SDR25 module...\n"
	rmmod nex_SD_SDR25.ko 
}



sdr50_mode_sdhc0()
{    
	echo -e "\n*** Loading SDR50 - 4bit module ***\n"
	insmod nex_SD_SDR50.ko sdhc=0
	sleep 2
	#echo -e "Unloading SDR50 module...\n"
	rmmod nex_SD_SDR50.ko 
}



sdr104_mode_sdhc0()
{    
	echo -e "\n*** Loading SDR104 - 4bit module ***\n"
	insmod nex_SD_SDR104.ko sdhc=0
	sleep 2
	#echo -e "Unloading SDR104 module...\n"
	rmmod nex_SD_SDR104.ko 
}



ddr50_mode_sdhc0()
{    
	echo -e "\n*** Loading DDR50 - 4bit module***\n"
	insmod nex_SD_DDR50.ko sdhc=0
	sleep 2
	#echo -e "Unloading DDR50 module...\n"
	rmmod nex_SD_DDR50.ko  
}



SD_3_0_CARD_SDHC0()
{

	echo -e "Select the Frequency modes for the SD3.0:\n
	0 => SDR12 \n 
	1 => SDR25 \n
	2 => SDR50 \n
	3 => SDR104 \n
	4 => DDR50 \n"
echo -e "??"
	read freqMode3;
	
	case $freqMode3 in
	
		0) 
			echo -e "SDR12 has been selected with default Buswidth as 4-bit\n"
			sdr12_mode_sdhc0
		;;
	
		1) 
			echo -e "SDR25 has been selected with default Buswidth as 4-bit\n"
			sdr25_mode_sdhc0
		;;

		2) 
			echo -e "SDR50 has been selected with default Buswidth as 4-bit\n"
			sdr50_mode_sdhc0
		;;

		3) 
			echo -e "SDR104 has been selected with default Buswidth as 4-bit\n"
			sdr104_mode_sdhc0
		;;

		4) 
			echo -e "DDR50 has been selected with default Buswidth as 4-bit\n"
			ddr50_mode_sdhc0
		;;
	esac
	
	
}


eMMC_fs25_mode_1bit_sdhc0()
{
	echo -e "\n*** Loading eMMC FullSpeed - 1bit module ***\n"
	insmod nex_MMC_FS25MHz.ko buswidth=1 sdhc=0
	sleep 2
	#echo -e "Unloading eMMC FullSpeed module...\n"
	rmmod nex_MMC_FS25MHz.ko  
}



eMMC_fs25_mode_4bit_sdhc0()
{
	echo -e "\n*** Loading eMMC FullSpeed - 4bit module ***\n"
	insmod nex_MMC_FS25MHz.ko buswidth=4 sdhc=0
	sleep 2
	#echo -e "Unloading eMMC FullSpeed module...\n"
	rmmod nex_MMC_FS25MHz.ko  
}



eMMC_fs25_mode_8bit_sdhc0()
{
	echo -e "\n*** Loading eMMC FullSpeed - 8bit module ***\n"
	insmod nex_MMC_FS25MHz.ko buswidth=8 sdhc=0
	sleep 2
	#echo -e "Unloading eMMC FullSpeed module...\n"
	rmmod nex_MMC_FS25MHz.ko  
}



eMMC_hs50_mode_1bit_sdhc0()
{
	echo -e "\n*** Loading eMMC HighSpeed - 1bit module ***\n"
	insmod nex_MMC_HS50MHz.ko buswidth=1 sdhc=0
	sleep 2
	#echo -e "Unloading eMMC hs50 module...\n"
	rmmod nex_MMC_HS50MHz.ko   
}



eMMC_hs50_mode_4bit_sdhc0()
{
	echo -e "\n*** Loading eMMC HighSpeed - 4bit module ***\n"
	insmod nex_MMC_HS50MHz.ko buswidth=4 sdhc=0
	sleep 2
	#echo -e "Unloading eMMC HighSpeed module...\n"
	rmmod nex_MMC_HS50MHz.ko   
}



eMMC_hs50_mode_8bit_sdhc0()
{
	echo -e "\n*** Loading eMMC HighSpeed - 8bit module ***\n"
	insmod nex_MMC_HS50MHz.ko buswidth=8 sdhc=0
	sleep 2
	#echo -e "Unloading eMMC HighSpeed module...\n"
	rmmod nex_MMC_HS50MHz.ko   
}


eMMC_ddr_mode_4bit_sdhc0()
{
	echo -e "\n*** Loading eMMC DDR - 4bit module ***\n"
	insmod nex_MMC_DDR.ko buswidth=4 sdhc=0
	sleep 2
	#echo -e "Unloading eMMC DDR module...\n"
	rmmod nex_MMC_DDR.ko   
}



eMMC_ddr_mode_8bit_sdhc0()
{
	echo -e "\n*** Loading eMMC DDR - 8bit module ***\n"
	insmod nex_MMC_DDR.ko buswidth=8 sdhc=0
	sleep 2
	#echo -e "Unloading eMMC DDR module...\n"
	rmmod nex_MMC_DDR.ko   
}



eMMC_hs200_mode_4bit_sdhc0()
{
	echo -e "\n*** Loading eMMC HS200 - 4bit module ***\n"
	insmod nex_MMC_SetHS200.ko buswidth=4 sdhc=0
	sleep 2
	#echo -e "Unloading eMMC HS200 module...\n"
	rmmod nex_MMC_SetHS200.ko   
}



eMMC_hs200_mode_8bit_sdhc0()
{
	echo -e "\n*** Loading eMMC HS200 - 8bit module ***\n"
	insmod nex_MMC_SetHS200.ko buswidth=8 sdhc=0
	sleep 2
	#echo -e "Unloading eMMC HS200 module...\n"
	rmmod nex_MMC_SetHS200.ko   
}



MMC_CARD_SDHC0()
{
	echo -e "Select the Frequency modes for the MMC_CARD:\n
	0 => Fullspeed \n 
	1 => HighSpeed \n"
echo -e "??"
	read freqMode5;
	case $freqMode5 in
	
		0) 
			echo -e "Select the Buswidth:\n
	1 => 1bit\n 
	4 => 4bit\n
	8 => 8bit\n"
echo -e "??"
			read buswidth3
			case $buswidth3 in
			
				1)
					echo "MMC FullSpeed has been selected with the Buswidth as 1-bit\n"
					eMMC_fs25_mode_1bit_sdhc0
				;;
				4)
					echo "MMC FullSpeed has been selected with the Buswidth as 4-bit\n"
					eMMC_fs25_mode_4bit_sdhc0
				;;
				8)
					echo "MMC FullSpeed has been selected with the Buswidth as 8-bit\n"
					eMMC_fs25_mode_8bit_sdhc0
				;;
			esac
		;;
	
		1)
			echo -e "Select the Buswidth:\n
	1 => 1bit\n 
	4 => 4bit\n
	8 => 8bit\n"
echo -e "??"
			read buswidth4
			case $buswidth4 in
			
				1)
					echo -e "MMC HighSpeed has been selected with the Buswidth as 1-bit\n"
					eMMC_hs50_mode_1bit_sdhc0
				;;
				4)
					echo -e "MMC HighSpeed has been selected with the Buswidth as 4-bit\n";
					eMMC_hs50_mode_4bit_sdhc0
				;;
				8)
					echo "MMC HighSpeed has been selected with the Buswidth as 8-bit\n"
					eMMC_hs50_mode_8bit_sdhc0
				;;
			esac
		;;
	esac
}




eMMC_CARD_SDHC0()
{
	echo -e "Select the Frequency modes for the eMMC_CARD:\n
	0 => Fullspeed \n 
	1 => HighSpeed \n
	2 => DDR \n
	3 => HS200 \n"
echo -e "??"
	read freqMode6;
	case $freqMode6 in
	
		0) 
			echo -e "Select the Buswidth:\n
	1 => 1bit\n 
	4 => 4bit\n
	8 => 8bit\n"
echo -e "??"
			read buswidth5
			case $buswidth5 in
			
				1)
					eMMC_fs25_mode_1bit_sdhc0
				;;
				4)
					eMMC_fs25_mode_4bit_sdhc0
				;;
				8)
					eMMC_fs25_mode_8bit_sdhc0
				;;
			esac		
		;;
		1)
			echo -e "Select the Buswidth:\n
	1 => 1bit\n 
	4 => 4bit\n
	8 => 8bit\n"
echo -e "??"
			read buswidth6
			case $buswidth6 in
			
				1)
					eMMC_hs50_mode_1bit_sdhc0
				;;
				4)
					eMMC_hs50_mode_4bit_sdhc0
				;;
				8)
					eMMC_hs50_mode_8bit_sdhc0
				;;
			esac
		;;

		2)
			echo -e "Select the Buswidth:\n
	4 => 4bit\n
	8 => 8bit\n"
echo -e "??"
			read buswidth7
			case $buswidth7 in
			
				4)
					eMMC_ddr_mode_4bit_sdhc0
				;;
				8)
					eMMC_ddr_mode_8bit_sdhc0
				;;
			esac
		;;

		3)
			echo -e "Select the Buswidth:\n
	4 => 4bit\n
	8 => 8bit\n"
echo -e "??"
			read buswidth8
			case $buswidth8 in
			
				4)
					eMMC_hs200_mode_4bit_sdhc0
				;;
				8)
					eMMC_hs200_mode_8bit_sdhc0
				;;
			esac
		;;
	esac
}


sdio_fs25_mode_1bit_sdhc0()
{    
	echo -e "\n*** Loading SDIO FullSpeed - 1bit module in HC0 ***\n"
	insmod nex_SDIO_FullSpeed25MHz.ko buswidth=1  sdhc=0
	sleep 2
	#echo -e "Unloading FullSpeed module...\n"
	rmmod nex_SDIO_FullSpeed25MHz.ko 
}
sdio_fs25_mode_4bit_sdhc0()
{    
	echo -e "\n*** Loading SDIO FullSpeed - 4bit module in HC0 ***\n"
	insmod nex_SDIO_FullSpeed25MHz.ko buswidth=4 sdhc=0
	sleep 2
	#echo -e "Unloading FullSpeed module...\n"
	rmmod nex_SDIO_FullSpeed25MHz.ko 
}

sdio_hs50_mode_1bit_sdhc0()
{    
	echo -e "\n*** Loading SDIO HighSpeed - 1bit module in HC0 ***\n"
	insmod nex_SDIO_HighSpeed50MHz.ko buswidth=1  sdhc=0
	sleep 2
	#echo -e "Unloading HighSpeed module...\n"
	rmmod nex_SDIO_HighSpeed50MHz.ko 
}
sdio_hs50_mode_4bit_sdhc0()
{    
	echo -e "\n*** Loading SDIO HighSpeed - 4bit module in HC0 ***\n"
	insmod nex_SDIO_HighSpeed50MHz.ko buswidth=4 sdhc=0
	sleep 2
	#echo -e "Unloading HighSpeed module...\n"
	rmmod nex_SDIO_HighSpeed50MHz.ko 
}

sdio_sdr12_mode_sdhc0()
{    
	echo -e "\n*** Loading SDIO SDR12 - 4bit module ***\n"
	insmod nex_SDIO_SDR12.ko  sdhc=0
	sleep 2
	#echo -e "Unloading SDR12 module...\n"
	rmmod nex_SDIO_SDR12.ko 
}



sdio_sdr25_mode_sdhc0()
{    
	echo -e "\n*** Loading SDIO SDR25 - 4bit module ***\n"
	insmod nex_SDIO_SDR25.ko sdhc=0
	sleep 2
	#echo -e "Unloading SDR25 module...\n"
	rmmod nex_SDIO_SDR25.ko 
}



sdio_sdr50_mode_sdhc0()
{    
	echo -e "\n*** Loading SDIO SDR50 - 4bit module ***\n"
	insmod nex_SDIO_SDR50.ko sdhc=0
	sleep 2
	#echo -e "Unloading SDR50 module...\n"
	rmmod nex_SDIO_SDR50.ko 
}



sdio_sdr104_mode_sdhc0()
{    
	echo -e "\n*** Loading SDIO SDR104 - 4bit module ***\n"
	insmod nex_SDIO_SDR104.ko sdhc=0
	sleep 2
	#echo -e "Unloading SDR104 module...\n"
	rmmod nex_SDIO_SDR104.ko 
}


sdio_ddr50_mode_sdhc0()
{    
	echo -e "\n*** Loading SDIO DDR50 - 4bit module ***\n"
	insmod nex_SD_DDR50.ko sdhc=0
	sleep 2
	#echo -e "Unloading DDR50 module...\n"
	rmmod nex_SD_DDR50.ko  
}


SDIO_2_0_CARD_SDHC0()
{

	echo -e "Select the Frequency modes for the SDIO2.0:\n
	0 => FullSpeed \n 
	1 => HighSpeed \n"
echo -e "??"

	read freqMode3;
	case $freqMode3 in
	
		0) 

			echo -e "Select the Buswidth:\n
	1 => 1bit \n
	4 => 4bit \n"
echo -e "??"

			read buswidth1
			case $buswidth1 in
			
				1)
					echo "SDIO Fullspeed mode has been selected with the Buswidth as 1-bit\n"
					sdio_fs25_mode_1bit_sdhc0
				;;
				4)
					echo "SDIO Fullspeed mode has been selected with the Buswidth as 4-bit\n"
					sdio_fs25_mode_4bit_sdhc0
				;;
			esac
		;;
	
		1) 
			echo -e "Select the Buswidth:\n
	1 => 1bit\n
	4 => 4bit\n:"
echo -e "??"
			read buswidth2
			case $buswidth2 in
			
				1)
					echo -e "SDIO HighSpeed mode has been selected with the Buswidth as 1-bit\n"
					sdio_hs50_mode_1bit_sdhc0
				;;
				4)
					echo -e "SDIO HighSpeed mode has been selected with the Buswidth as 4-bit\n"
					sdio_hs50_mode_4bit_sdhc0
				;;
			esac
		;;

		
	esac
}
SDIO_3_0_CARD_SDHC0()
{

	echo -e "Select the Frequency modes for the SDIO3.0:\n
	0 => SDR12 \n 
	1 => SDR25 \n
	2 => SDR50 \n
	3 => SDR104 \n
	4 => DDR50 \n"
echo -e "??"
	read freqMode3;
	case $freqMode3 in
	
		0) 
			echo -e "SDIO SDR12 has been selected with default Buswidth as 4-bit\n"
			sdio_sdr12_mode_sdhc0
		;;
	
		1) 
			echo -e "SDIO SDR25 has been selected with default Buswidth as 4-bit\n"
			sdio_sdr25_mode_sdhc0
		;;

		2) 
			echo -e "SDIO SDR50 has been selected with default Buswidth as 4-bit\n"
			sdio_sdr50_mode_sdhc0
		;;

		3) 
			echo -e "SDIO SDR104 has been selected with default Buswidth as 4-bit\n"
			sdio_sdr104_mode_sdhc0
		;;

		4) 
			echo -e "SDIO DDR50 has been selected with default Buswidth as 4-bit\n"
			sdio_ddr50_mode_sdhc0
		;;
	esac
}



#################################FUNCTION DEFINITION ENDS##################################





>/var/log/messages


echo -e "\n*** Loading OSAL Module *** \n"
insmod nex_osal.ko

echo -e "\n*** Loading PCI Module *** \n"
insmod nex_pci.ko

echo -e "\n Select Debug Level (From 0 to 5)\n "
echo -e "??"
read log
echo -e "Select Transfer Mode 
	0 => PIO \n
	1 => DMA \n
	2 => ADMA \n"
echo -e "??"
read dma

echo -e "\n*** Loading Driver Module *** \n"
insmod nex_driver.ko debug_level=$log dma_level=$dma
pdma=$dma

echo -e "\n Enter the card inserted in Host Controller  ... \n
	        0 = > SD2.0 CARD \n 
	        1 = > SD3.0 CARD \n
	        2 = > eSD CARD \n
	        3 = > MMC CARD \n
	        4 = > eMMC CARD \n
	        5 = > SDIO2.0 CARD \n
		6 = > SDIO3.0 CARD \n"
echo -e "??"
read sdhc0_card

sdio0=0

case $sdhc0_card in 
	 0) 
		echo -e "\n*** Loading Block Driver Module *** \n"
		insmod nex_block.ko dmaMode=$dma
		SD_2_0_CARD_SDHC0;;
		
	 1) 
		echo -e "\n*** Loading Block Driver Module *** \n"
		insmod nex_block.ko dmaMode=$dma
		SD_3_0_CARD_SDHC0;;

	 2) 
		echo -e "\n*** Loading Block Driver Module *** \n"
		insmod nex_block.ko dmaMode=$dma
		eSD_CARD_SDHC0;;

	 3) 
		echo -e "\n*** Loading Block Driver Module *** \n"
		insmod nex_block.ko dmaMode=$dma
		MMC_CARD_SDHC0;;

	 4) 
		echo -e "\n*** Loading Block Driver Module *** \n"
		insmod nex_block.ko dmaMode=$dma
		eMMC_CARD_SDHC0;;

	 5) 	sdio0=1
		SDIO_2_0_CARD_SDHC0;;

	 6 ) 	sdio0=1
		SDIO_3_0_CARD_SDHC0;;
esac




if [ $sdio0 -eq 0 ]
then
	data_transfer_sdhc0
else
	echo -e "Doing Data transfer with CMD53 in HC0 card\n"
	echo -e "\n*** Loading SDIO CMD53 module ***\n"
	insmod nex_SDIO_CMD53.ko sdhc=0 dmaMode=$dma
	sleep 2
	#echo -e "Unloading SDIO CMD53 module...\n"
	rmmod nex_SDIO_CMD53.ko
fi


while :
do
      echo -e "\n\nDo you want to continue? [yes or no]: "
echo -e "??"
      read yno
      case $yno in
                [yY] | [yY][Ee][Ss] ) 
			echo -e "Select Transfer Mode 
	0 => PIO \n
	1 => DMA \n
	2 => ADMA \n"
echo -e "??"

				read dma
				if [ $sdio0 -eq 0 ] 
				then
					if [ $pdma -eq $dma ]
					then 
						echo -e "Transfer Mode not changed. Its same as previous selection. So not removing block driver\n"
					else
						echo - e "Unloading the block driver since Transfer Mode has been changed\n"
						rmmod nex_block.ko
						sleep 2
						echo -e "*** Loading Block driver with TransferMode-$dma *** \n"
						insmod nex_block.ko dmaMode=$dma
						pdma=$dma
					fi
				fi
		
				case $sdhc0_card in 
					 0) SD_2_0_CARD_SDHC0;;	
					 1) SD_3_0_CARD_SDHC0;;
					 2) eSD_CARD_SDHC0;;
					 3) MMC_CARD_SDHC0;;
					 4) eMMC_CARD_SDHC0;;
					 5) SDIO_2_0_CARD_SDHC0;;
					 6) SDIO_3_0_CARD_SDHC0;;
				esac

				if [ $sdio0 -eq 0 ]
				then
					data_transfer_sdhc0
				else
					echo -e "Doing Data transfer with CMD53 in HC0 card\n"
					echo -e "\n*** Loading SDIO CMD53 module with TransferMode-$dma ***\n"
					insmod nex_SDIO_CMD53.ko sdhc=0 dmaMode=$dma
					sleep 2
					#echo -e "Unloading SDIO CMD53 module...\n"
					rmmod nex_SDIO_CMD53.ko
				fi

		;;
        	[nN] | [n|N][O|o] )
				if [ $sdio0 -eq 0 ] || [ $sdio1 -eq 0 ]
				then
					echo -e "\n Unloading Block Driver Module\n"
					rmmod nex_block.ko
				fi
				echo -e "\n Unloading Driver Module... \n"
		                rmmod nex_driver.ko
	                        echo -e "\n Unloading PCI Module... \n"
		                rmmod nex_pci.ko
	                        echo -e "\n Unloading OSAL Module... \n"
		                rmmod nex_osal.ko
		                exit 0
                
                ;;
	 	*) 
				echo -e "Invalid input"
				if [ $sdio0 -eq 0 ] 
				then
					echo -e "\n Unloading Block Driver Module\n"
					rmmod nex_block.ko
				fi
	                        echo -e "\n Unloading Driver Module... \n"
		                rmmod nex_driver.ko
	                        echo -e "\n Unloading PCI Module... \n"
		                rmmod nex_pci.ko
	                        echo -e "\n Unloading OSAL Module... \n"
		                rmmod nex_osal.ko
		                dmesg -c
	                        exit 0
            ;;
	esac
done




