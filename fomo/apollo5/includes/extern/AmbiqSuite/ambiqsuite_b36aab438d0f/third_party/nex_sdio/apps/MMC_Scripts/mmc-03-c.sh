#! /bin/sh
dmesg -c
echo -e "033[34m Please enter number of Iterations \033[0m\n"
read n
for ((i=1 ;i<=n ;i++))
	do
	echo -e "033[33m Iteration no.$i \033[0m\n"
	echo -e "033[31m mounting the mmccard \033[0m\n"
	mount -t ext2 /dev/BlockDev0p1 /mnt/mmccard/
	df -h
	echo -e "033[31m Copying the file \033[0m\n"
	cp FILE_1MB.txt /mnt/mmccard/test.txt
	ls -al /mnt/mmccard/
	df -h
	sleep 5
	rm -rf /mnt/mmccard/FILE_1MB.txt
	echo -e "033[31mfile removed \033[0m\n"
	sleep 10
	ls -al /mnt/mmccard
	df -h
	echo "Unmounting the mmccard"
	umount -l /mnt/mmccard
	sleep 10
	df -h
	done
if [ -d ../../../TestCaptures/$1 ] 
then 
 echo "dir already exists"
else
 mkdir ../../../TestCaptures/$1
fi
dmesg >> ../../../TestCaptures/$1/mmc_3-c.txt
