dmesg -c
echo "please enter the partion size of the card"
read size
n=$size/5
for ((i=1 ;i<n;i++))
	do
	echo " Running for $i time"
	df -h
	cp FILE_1MB.txt /mnt/mmccard/test$i.txt
	echo "file copied"
	ls -al /mnt/mmccard/
	df -h
	done
if [ -d ../../../TestCaptures/$1 ] 
then 
 echo "dir already exists"
else
 mkdir ../../../TestCaptures/$1
fi
dmesg >> ../../../TestCaptures/$1/mmc-4-a.txt
