#!/bin/bash
# This is the script to launch the flashbin program
# By Damien Gros & Gabriel Salles-Loustau
# Requierements : udev, sed, awk, block2mtd module, mtd-tools package.
# Optionnal : ubifs kernel, logfs patched kernel.

# Cle a Damien
#"H6LOD3Q9YL925WB0"
# 0700077A12A10233

flashbin_configfile="/home/utilisateur/workspace/flashbin_kiki/flashbin/flashbin.conf"
flashbin_logfile="/home/utilisateur/workspace/flashbin_kiki/flashbin/flashbin.log"

# detect the key with udev
flashbin_run()
{
echo "Detecting the key..."
read_serial=`sed -n '/\[serial\]/,/\[\/serial\]/{//d;p}' $flashbin_configfile`
variable=`ls /sys/block | grep sd?*`
set $variable

serial=1

while [ -n "$1" ]
do

	serial=`/usr/bin/udevinfo -a -p /sys/block/$1 | grep -m1 serial | cut -d'=' -f3 | cut -d'"' -f2`

                if  [ "$serial" = "$read_serial" ]
                then
                        echo "Key found, serial = $read_serial"
               			flashbin_device=$1
                        export $flashbin_device
               			break
                else 
                		flashbin_device="erreur"
                fi
    shift 1
done

if [ $flashbin_device = "erreur" ]
then
	echo "No key has been detected"
else
	echo "Key detected as $flashbin_device"
	create_dev $flashbin_device
fi


}


# Create the devices mtdX and mtdblockX used by logfs, jffs2 and ubi filesystems
create_dev()
{

device=$1

# Create the devices in /dev 
echo "Emulating mtd devices for your block devices"

echo "test 1"
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
echo "test2"

partitions=`sed -n '/\[partitions\]/,/\[\/partitions\]/{//d;p}' $flashbin_configfile | awk '{ print $1 }' | tr '\n' ' ' `

set $partitions

for i
do
modprobe_parameters="$modprobe_parameters block2mtd=/dev/$device$i"
done

echo "$modprobe_parameters"
modprobe block2mtd $modprobe_parameters
echo "$?"
flashbin_mount $device

}

# mount filesystem
flashbin_mount()
{
device=$1

flashbin_mt_devices=`dmesg | tail -n10 | grep block2mtd | grep erase_size | grep "\[d:" | cut -d":" -f2 | cut -d" " -f2 | tr '\n' ' '`
#echo "MTD = $flashbin_mt_devices"

flashbin_jffs2_table=`dmesg | tail -n10 | grep block2mtd | grep erase_size | grep "\[d:" | cut -d":" -f2 | cut -b5 | tr '\n' ' '`
#echo "jffs3 table = $flashbin_jffs2_table"

mount_point=`sed -n '/\[partitions\]/,/\[\/partitions\]/{//d;p}' $flashbin_configfile | awk '{ print $2 }' | tr '\n' ' ' `
#echo "point de montage = $mount_point"

partitions=`sed -n '/\[partitions\]/,/\[\/partitions\]/{//d;p}' $flashbin_configfile | awk '{ print $1 }' | tr '\n' ' ' `
#echo "partitions = $partitions"

partitions_type=`sed -n '/\[partitions\]/,/\[\/partitions\]/{//d;p}' $flashbin_configfile | awk '{ print $3 }' | tr '\n' ' ' `
#echo "partition type = $partitions_type "

flashbin_rbind_table=`sed -n '/\[rbind_table\]/,/\[\/rbind_table\]/{//d;p}' $flashbin_configfile | awk '{ print $2 }' | tr '\n' ' ' `

dup="dup"
compteur=0

for j in $partitions_type
do
compteur=`expr $compteur + 1`
case "$j" in
     logfs)
          flashbin_partition=`echo $partitions | cut -d" " -f$compteur`
          flashbin_mountpoint=`echo $flashbin_rbind_table | cut -d" " -f$compteur`
          flashbin_rbind_mountpoint=`echo $mount_point | cut -d" " -f$compteur`
	  echo "logfs: $flashbin_mountpoint $flashbin_partition $flashbin_rbind_mountpoint"
          echo " rbind parm : $flashbin_rbind_mountpoint $flashbin_mountpoint$dup"
#          mount --rbind $flashbin_rbind_mountpoint $flashbin_mountpoint$dup 
#	  mount -t logfs $flashbin_partition $flashbin_mountpoint && mount --rbind $flashbin_mountpoint $flashbin_rbind_mountpoint &
     ;;
     jffs2)
          flashbin_mtdblock=`echo $flashbin_jffs2_table | cut -d" " -f $compteur`
          flashbin_partition="/dev/mtdblock$flashbin_mtdblock"
          flashbin_mountpoint=`echo $flashbin_rbind_table | cut -d" " -f $compteur`
          flashbin_rbind_mountpoint=`echo $mount_point | cut -d" " -f$compteur`
          echo "jffs2: $flashbin_partition $flashbin_mountpoint $flashbin_rbind_mountpoint"
          echo " rbind parm : $flashbin_rbind_mountpoint $flashbin_mountpoint$dup"
#          mount --rbind $flashbin_rbind_mountpoint $flashbin_mountpoint$dup 
#	  mount -t jffs2 $flashbin_partition $flashbin_mountpoint && mount --rbind $flashbin_mountpoint $flashbin_rbind_mountpoint &
     ;;
     ubifs)

     ;;
esac
done

flashbin_synchronize

}

flashbin_configure()
{
mkdir -p `sed -n '/\[rbind_table\]/,/\[\/rbind_table\]/{//d;p}' $flashbin_configfile | awk '{ print $2 }' | tr '\n' ' ' `
return 0
}

flashbin_synchronize()
{
flashbin_synchronize_flag=`grep "synchronized" $flashbin_logfile | cut -d"=" -f2`
if [ "$flashbin_synchronize_flag" = yes ]
then

   return 0

else
     flashbin_synchronize_way=`grep "way" $flashbin_logfile | cut -d"=" -f2`
     flashbin_path_to_synchonize=`sed -n '/\[paths\]/,/\[\/paths\]/{//d;p}' $flashbin_logfile | grep 1 | awk '{ print $1 }' | tr '\n' ' ' `
#     flashbin_rbind_table=`sed -n '/\[rbind_table\]/,/\[\/rbind_table\]/{//d;p}' $flashbin_configfile | awk '{ print $2 }' | tr '\n' ' ' `
     if [ "$flashbin_synchronize_way"="flashtodisk" ]
     then
         for i in $flashbin_path_to_synchonize
         do
    		# cp -ru  `sed -n '/\[rbind_table\]/,/\[\/rbind_table\]/{//d;p}' $flashbin_configfile | grep "$i" | awk '{ print $2 }'` `sed -n '/\[rbind_table\]/,/\[\/rbind_table\]/{//d;p}' $flashbin_configfile | grep "$i" | awk '{ print $2 }'`$dup 
		 plop=`sed -n '/\[rbind_table\]/,/\[\/rbind_table\]/{//d;p}' $flashbin_configfile | grep "$i" | awk '{ print $2 }'`$dup
                 echo " PLOP $plop"
	 done
     elif [ "$flashbin_synchronize_way"="disktoflash" ]
     then
	 for i in $flashbin_path_to_synchonize
	 do
		 echo ""
#	     cp -ru `sed -n '/\[rbind_table\]/,/\[\/rbind_table\]/{//d;p}' $flashbin_configfile | grep "$i" | awk '{ print $2 }'` $i
         done
     fi
fi

./flashbin flash

}

flashbin_status()
{
return 0
}



if [ "$(id -u)" != "0" ]; then
    echo "you should run this script as root"
    exit 1
    
else
case "$1" in 
     start)
		flashbin_run
     ;;
     synchonize)
        flashbin_synchronize
     ;;
     status)
        flashbin_status
     ;;
    configure)
        flashbin_configure
     ;;
     *)
        echo "FlashBin v0.01"
        echo "Usage : [start|synchronize|status|configure]"
     ;;
esac
fi
 
