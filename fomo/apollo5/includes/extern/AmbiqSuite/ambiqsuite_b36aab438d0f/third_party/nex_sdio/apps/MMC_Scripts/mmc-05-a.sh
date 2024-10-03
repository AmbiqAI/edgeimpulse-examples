dmesg -c
for i in 0
	do
	echo " Running for $i time for SD/MMC"
	insmod nex_driver.ko debug_level=3
	echo "Inserted the nex bus driver"
	sleep 5
	insmod nex_block.ko
	echo "Inserted the block driver"
	sleep 5
	fdisk -l /dev/BlockDev0
	sleep 3
	echo "Formating the card and creating a filesystems"
	mkfs -t ext2 /dev/BlockDev0p1
	sleep 20
	if [ -d /mnt/mmccard ] 
	 then 
		echo " mmccard dir already exists"
	 else
		mkdir /mnt/mmccard
	fi
	df -h
	mount -t ext2 /dev/BlockDev0p1 /mnt/mmccard
	echo " MMC card is mounted"
	df -h
	cp FILE_1MB.txt /mnt/mmccard/test.txt
	echo "file copied"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	umount /mnt/mmccard
	echo "Card has been unmounted"
	df -h
	rmmod nex_block
	echo "Removed the nex bus driver\n"
	sleep 2
	rmmod nex_driver
	echo "Removed the nex bus driver"
	done
if [ -d ../../../TestCaptures/$1 ] 
then 
 echo "dir already exists"
else
 mkdir ../../../TestCaptures/$1
fi
dmesg >> ../../../TestCaptures/$1/mmc_5-a.txt


