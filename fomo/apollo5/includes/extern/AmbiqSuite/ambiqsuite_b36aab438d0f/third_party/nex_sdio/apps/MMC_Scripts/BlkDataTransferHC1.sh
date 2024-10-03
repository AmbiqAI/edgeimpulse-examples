#/bin/bash

echo -e "** Doing Data Transfer **\n"

if [ -d /mnt/card2 ] 
        then 
	        echo "card2 dir already exists"
        else
	        mkdir /mnt/card2
         fi

if [ -d /home/card2_read ] 
        then 
	        echo "card2_read dir already exists"
        else
	        mkdir /home/card2_read
         fi

echo "\tMounting the device BlockDev10p1 of HC1...\n";
mount -t ext2 /dev/BlockDev10p1 /mnt/card2/
sleep 5;

echo "\nCopy the files to be transferred to the card2 in /apps folder of the stack\n"
echo "\tEnter the filename to be written to the card in HC1...\n";
read FILENAME2 

WR_PATH2="/mnt/card2/$FILENAME2"
RD_PATH2="/home/card2_read/$FILENAME2"


echo -e "\n Writing $FILENAME2 to Card2:\n"
cp $FILENAME2 $WR_PATH2 
sync
sleep 10

echo -e "\n Reading $FILENAME2 from Card2:\n"
cp $WR_PATH2 $RD_PATH2 
sync
sleep 10

echo -e "\n Comparing file read from Card2 - ($RDPATH2) with Original file - (/apps/$FILENAME2) :\n"
cmp $FILENAME2 $RD_PATH2 
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
         $WR_PATH2 \n
         $RD_PATH2 \n"

rm -f $WR_PATH2
rm -f $RD_PATH2

echo -e "\n Unmounting card2 \n"
umount /dev/BlockDev10p1


echo -e "** Completed Data Transfer in HC1 card **\n"
