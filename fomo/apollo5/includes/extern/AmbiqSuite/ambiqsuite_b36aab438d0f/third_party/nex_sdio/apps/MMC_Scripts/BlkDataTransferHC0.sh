#/bin/bash

echo -e "** Doing Data Transfer **\n"
if [ -d /mnt/card1 ] 
        then 
	        echo "card1 dir already exists"
        else
	        mkdir /mnt/card1
fi


if [ -d /home/card1_read ] 
        then 
	        echo "card1_read dir already exists"
        else
	        mkdir /home/card1_read
fi


echo "\tMounting the device BlockDev00p1 of HC0...\n";
mount -t ext2 /dev/BlockDev00p1 /mnt/card1/
sleep 5;

echo "\nCopy the files to be transferred to the card1 in /apps folder of the stack\n"
echo "\tEnter the filename to be written to the card in HC0...\n";
read FILENAME1 

WR_PATH1="/mnt/card1/$FILENAME1"
RD_PATH1="/home/card1_read/$FILENAME1"

echo -e "\n Writing $FILENAME1 to Card1:\n"
cp $FILENAME1 $WR_PATH1 
sync
sleep 10


echo -e "\n Reading $FILENAME1 from Card1:\n"
cp $WR_PATH1 $RD_PATH1 
sync
sleep 10


echo -e "\n Comparing file read from Card1 - ($RDPATH1) with Original file - (/apps/$FILENAME1) :\n"
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

echo -e "\n Unmounting card1 \n"
umount /dev/BlockDev00p1

echo -e "** Completed Data Transfer in HC0 card **\n"
