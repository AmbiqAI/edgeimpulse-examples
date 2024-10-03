dmesg -c
for i in 0
	do
	echo " Running for $i time for SD/MMC"
	insmod nex_driver.ko debug_level=3
	echo "Inserted the nex bus driver"
	lsmod
	sleep 5
	insmod nex_block.ko
	echo "Inserted the block driver in DMA mode"
	lsmod
	sleep 5
	fdisk -l /dev/BlockDev0
	sleep 3
	echo "Formatinng the card and creating a filesystems"
	mkfs -t ext2 /dev/BlockDev0p1
	sleep 10
	if [ -d /mnt/mmccard ] 
	 then 
		echo " mmccard dir already exists"
	 else
		mkdir /mnt/mmccard
	fi
	df -h
	mount -t ext2 /dev/BlockDev0 /mnt/mmccard
	echo " MMC card is mounted"
	df -h
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in default mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in default mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	insmod test_1bit.ko
	echo "********Inserted the 1bit test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 1bit mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 1bit mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_1bit.ko
	echo "*******Rmoved the 1bit test module***********"
	insmod test_4bit.ko
	echo "********Inserted the 4bit test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 4bit mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 4bit mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_4bit.ko
	echo "*******Rmoved the 4bit test module***********"
	insmod test_8bit.ko
	echo "********Inserted the 8bit test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 8bit mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 8bit mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_8bit.ko
	echo "*******Rmoved the 8bit test module***********"
	insmod test_12MHz.ko
	echo "********Inserted the 12MHz test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 12MHz mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 12MHz mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_12MHz.ko
	echo "*******Rmoved the 12MHz test module***********"
	insmod test_24MHz.ko
	echo "********Inserted the 24MHz test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 24MHz mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 24MHz mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_24MHz.ko
	echo "*******Rmoved the 24MHz test module***********"
	insmod test_48MHz.ko
	echo "********Inserted the 48MHz test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 48MHz mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 48MHz mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_48MHz.ko
	echo "*******Rmoved the 48MHz test module***********"
	insmod test_1bit12MHz.ko
	echo "********Inserted the 1bit12MHz test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 1bit12MHz mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 1bit12MHz mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_1bit12MHz.ko
	echo "*******Rmoved the 1bit12MHz test module***********"
	insmod test_1bit24MHz.ko
	echo "********Inserted the 1bit24MHz test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 1bit24MHz mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 1bit24MHz mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_1bit24MHz.ko
	echo "*******Rmoved the 1bit24MHz test module***********"
	insmod test_1bit48MHz.ko
	echo "********Inserted the 1bit48MHz test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 1bit48MHz mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 1bit48MHz mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_1bit48MHz.ko
	echo "*******Rmoved the 1bit48MHz test module***********"
	insmod test_4bit12MHz.ko
	echo "********Inserted the 4bit12MHz test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 4bit12MHz mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 4bit12MHz mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_4bit12MHz.ko
	echo "*******Rmoved the 4bit12MHz test module***********"
	insmod test_4bit24MHz.ko
	echo "********Inserted the 4bit24MHz test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 4bit24MHz mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 4bit24MHz mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_4bit24MHz.ko
	echo "*******Rmoved the 4bit24MHz test module***********"
	insmod test_4bit48MHz.ko
	echo "********Inserted the 4bit48MHz test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 4bit48MHz mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 4bit48MHz mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_4bit48MHz.ko
	echo "*******Rmoved the 4bit48MHz test module***********"
	insmod test_8bit12MHz.ko
	echo "********Inserted the 8bit12MHz test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 8bit12MHz mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 8bit12MHz mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_8bit12MHz.ko
	echo "*******Rmoved the 8bit12MHz test module***********"
	insmod test_8bit24MHz.ko
	echo "********Inserted the 8bit24MHz test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 8bit24MHz mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 8bit24MHz mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_8bit24MHz.ko
	echo "*******Rmoved the 8bit24MHz test module***********"
	insmod test_8bit48MHz.ko
	echo "********Inserted the 8bit48MHz test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in 8bit48MHz mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in 8bit48MHz mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_8bit48MHz.ko
	echo "*******Rmoved the 8bit48MHz test module***********"
	insmod test_blksize.ko
	echo "********Inserted the blksize test module********"
	lsmod
	sleep 5
	cp ../../../TestScripts/FILE_1MB.txt /mnt/mmccard/test.txt
	cp /mnt/mmccard/test.txt ../../../TestScripts/FILE_1MB.txt
	echo "file copied in blksize mode"
	ls -al /mnt/mmccard/
	df -h
	sleep 3
	rm -rf /mnt/mmccard/test.txt
	echo "file removed in blksize mode"
	ls -al /mnt/mmccard
	sleep 5
	df -h
	rmmod test_blksize.ko
	echo "*******Rmoved the blksize test module***********"
	umount /mnt/mmccard
	echo "Card has been unmounted"
	df -h
	lsmod
	rmmod nex_block
	echo "Removed the nex bus driver\n"
	sleep 2
	lsmod
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


