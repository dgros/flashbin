#!/bin/bash
# This is the script to launch the flashbin program
# By Damien Gros & Gabriel Salles-Loustau
# Requierements : udev, sed, awk, block2mtd module, mtd-tools package.
# Optionnal : ubifs kernel, logfs patched kernel.

# Clé a Damien
#"H6LOD3Q9YL925WB0"
#0700077A12A10233

configfile="/home/gabriel/workspace/flashbin/flashbin.conf"

# detect the key with udev
udev_dem()
{
echo "Detecting the key"
read_serial=`sed -n '/\[serial\]/,/\[\/serial\]/{//d;p}' $configfile`
variable=`ls /sys/block | grep sd?*`
set $variable

serial=1

while [ -n "$1" ]
        do
        serial=`/usr/bin/udevinfo -a -p /sys/block/$1 | grep -m1 serial | cut -d'=' -f3 | cut -d'"' -f2`
                        if [ -z $serial ]
                                then
                                        serial="erreur"
                        fi
                        if  [ $serial = $read_serial ]
                                then
                                        echo "Key found, serial = $read_serial"
                                        create_dev $1
#                                        break;
                        fi
                shift 1
        done


#echo "$1"
echo "End of key détection"
}


# Create the devices mtdX and mtdblockX used by logfs, jffs2 and ubi filesystems
create_dev()
{

device=$1

# Create the devices in /dev 
echo "Emulating mtd devices for your block devices"

for a in `seq 0 16` ; do
	mknod /dev/mtd$a c 90 `expr $a + $a`
	mknod /dev/mtdblock$a b 31 $a
done	


partitions=`sed -n '/\[partitions\]/,/\[\/partitions\]/{//d;p}' $configfile | awk '{ print $1 }' | tr '\n' ' ' `

set $partitions

for i
do
modprobe_parameters="$modprobe_parameters block2mtd=/dev/$device$i"
echo $modprobe_parameters
done

modprobe block2mtd $modprobe_parameters

}

if [ "$(id -u)" != "0" ]; then
    echo "you should run this script as root"
    exit 1
    
else
	udev_dem
fi
