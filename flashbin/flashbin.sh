#!/bin/bash
# This is the script to launch the flashbin program
# By Damien Gros & Gabriel Salles-Loustau
# Requierements : udev, sed, awk, block2mtd module, mtd-tools package.
# Optionnal : ubifs kernel, logfs patched kernel.

# Cle a Damien
#"H6LOD3Q9YL925WB0"
# 0700077A12A10233

configfile="/home/gabriel/workspace/flashbin/flashbin.conf"

# detect the key with udev
flashbin_run()
{
echo "Detecting the key..."
read_serial=`sed -n '/\[serial\]/,/\[\/serial\]/{//d;p}' $configfile`
variable=`ls /sys/block | grep sd?*`
set $variable

serial=1

while [ -n "$1" ]
do
	serial=`/usr/bin/udevinfo -a -p /sys/block/$1 | grep -m1 serial | cut -d'=' -f3 | cut -d'"' -f2`
#                        if [ -z $serial ]
#                                then
#                                        serial="erreur"
#					detect_flag="erreur"
#					echo "$serial   $detect_flag"
#                        fi
                if  [ $serial = $read_serial ]
                then
                        echo "Key found, serial = $read_serial"
			detect_flag=$1
			break
		else 
			detect_flag="erreur"
                fi
        shift 1
done

if [ $detect_flag = "erreur" ]
then
	echo "No key has been detected"
else
	echo "Key detected as $detect_flag"
	create_dev $detect_flag
fi


}


# Create the devices mtdX and mtdblockX used by logfs, jffs2 and ubi filesystems
create_dev()
{

device=$1

# Create the devices in /dev 
echo "Emulating mtd devices for your block devices"

for a in `seq 0 16` ; do

	if [ ! -c /dev/mtd$a ]
	then
		mknod /dev/mtd$a c 90 `expr $a + $a`
	fi

	if [ ! -b /dev/mtdblock$a ]
	then
		mknod /dev/mtdblock$a b 31 $a
	fi

done	


partitions=`sed -n '/\[partitions\]/,/\[\/partitions\]/{//d;p}' $configfile | awk '{ print $1 }' | tr '\n' ' ' `

set $partitions

for i
do
modprobe_parameters="$modprobe_parameters block2mtd=/dev/$device$i"
done

modprobe block2mtd $modprobe_parameters

flashbin_mount

}

# mount filesystem
flashbin_mount()
{
partitions=`sed -n '/\[partitions\]/,/\[\/partitions\]/{//d;p}' $configfile | awk '{ print $1 }' | tr '\n' ' ' `
partitions_type=`sed -n '/\[partitions\]/,/\[\/partitions\]/{//d;p}' $configfile | awk '{ print $3 }' | tr '\n' ' ' `
mount_point=`sed -n '/\[partitions\]/,/\[\/partitions\]/{//d;p}' $configfile | awk '{ print $2 }' | tr '\n' ' ' `

#mount --rbind 

}

flashbin_configure()
{
return 0
}

flashbin_status()
{
return 0
}



if [ "$(id -u)" != "0" ]; then
    echo "you should run this script as root"
    exit 1
    
else
        if [ "$1"="" ]
	then
		flashbin_run
	elif [ "$1"="configure" ]
	then
		flashbin_configure
	elif [ "$1"="status" ]
	then
		flashbin_status
	fi
fi
