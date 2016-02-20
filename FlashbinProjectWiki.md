# Introduction #

This project aims to improve the boot time of linux distributions thanks to an USB flash key.
It also improves applications launch time.

# TODO #

- Script to format the key and make the first copy.

- Recode the modification.sh script in C and include it in the daemon.

- Create an Udev rule to start/stop the program.

# Installation #

- get the sources of the projet (Source section)

- run make && su -c "make install"

- format your usb key and create 3 partition. (ex : for a 2GB key uses 1,3 GB for /usr/lib, 500 for /usr/bin and the left space for /var/lib. Of course, this depends of your disk usage...)

- make the first copy of the folders you want to faster access. (ex = to accelerate the access to /usr/lib using the /dev/sdb2 partition of your USB key do

mount /dev/sdb2 /mnt/somemountpoint

cp -r /usr/lib/"asterisk" /mnt/somemountpoint

umount /dev/sdb2

)

- edit the /etc/flashbin.conf

> - in the serial section put the serial number of your key ("udevinfo -a -p /sys/block/sdb2" to get some info )

> - in the partitions section put the correspondence between the partition number of your key and the folder you have copied

- finally run the flashbin.sh script first with the "configure" option, then with the "start" option

# WARNING #

- We recommend you NOT to unplug your usb key while the program is running...We have not implemented anything for this situation yet...