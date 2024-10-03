dmesg -c
ix=0
echo -e "033[34m Please enter number of Iterations \033[0m\n"
read iy
while [ $ix -le $iy ]
  do
	echo -e "033[33m running loop for $ix time \033[0m\n"
	insmod nex_driver.ko
	echo -e "033[31m Loaded the Base driver \033[0m\n"
	sleep 3
	insmod nex_block.ko
	echo -e "033[31m Loaded the Block driver \033[0m\n"
	sleep 3
	rmmod nex_block	
	echo -e "033[31m Unloaded the block driver \033[0m\n"
	rmmod nex_driver
	echo -e "033[31m Unloaded the Base driver \033[0m\n"
	ix=$(($ix+1))
  done
if [ -d ../../../TestCaptures/$1 ] 
  then 
 	echo "dir already exists"
  else
 	mkdir ../../../TestCaptures/$1
fi
dmesg >> ../../../TestCaptures/$1/mmc_1-c.txt
