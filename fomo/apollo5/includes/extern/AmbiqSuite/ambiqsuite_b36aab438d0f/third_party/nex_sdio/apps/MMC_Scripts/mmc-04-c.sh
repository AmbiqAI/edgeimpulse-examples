for i in 0 1 2 
	do
	echo " Running for $i time"
	df -h
	cp FILE_1MB.txt /mnt/mmccard/File_1mb.txt
	echo "file copied"
	ls -al /mnt/mmccard/
	df -h
	rm -rf /mnt/mmccard/File_1mb.txt
	echo "file removed"
	ls -al /mnt/mmccard
	df -h
	done
if [ -d ../../../TestCaptures/$1 ] 
then 
 echo "dir already exists"
else
 mkdir ../../../TestCaptures/$1
fi
dmesg >> ../../../TestCaptures/$1/mmc_4-c.txt
