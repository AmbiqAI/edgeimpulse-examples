dmesg -c
echo "please enter the partion size of the card"
read size
n=$size/5
for ((j=1;j<n;j++))
	do
	rm -rf /mnt/mmccard/test$j.txt
	echo "file removed"
	sleep 5
	ls -al /mnt/mmccard
	df -h
	done	
if [ -d ../../../TestCaptures/$1 ] 
then 
 echo "dir already exists"
else
 mkdir ../../../TestCaptures/$1
fi
dmesg >> ../../../TestCaptures/$1/mmc-4-b.txt
