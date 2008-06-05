#!/bin/bash
# This is the script to launch the flashbin program
# By Damien Gros & Gabriel Salles-Loustau
# Requierements : udev, sed, awk
# Optionnal :  block2mtd module, mtd-tools package. ubifs kernel, logfs patched kernel.

flashbin_configfile="/etc/flashbin.conf"
flashbin_logfile="/var/log/flashbin.log"
start_daemon="/usr/local/bin/flashbin"
synchronisation_run_file="/var/run/synchonisation.run"

export flag_key="false"
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
			export     flag_key="true"
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
   cat $flashbin_configfile | grep jffs2; cat $flashbin_configfile | grep logfs; cat $flashbin_configfile | grep ubifs > /dev/null
   if [ $? -ne 0 ]
   then
     echo "No special filesystem, skipping create_dev()"
     flashbin_mount $flashbin_device
   else
	 create_dev $flashbin_device
   fi
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

partitions=`sed -n '/\[partitions\]/,/\[\/partitions\]/{//d;p}' $flashbin_configfile | awk '{ print $1 }' | tr '\n' ' ' `

set $partitions

for i
do
modprobe_parameters="$modprobe_parameters block2mtd=/dev/$device$i"
done

modprobe block2mtd $modprobe_parameters

flashbin_mount $device

}

# mount filesystem
flashbin_mount()
{
device=$1

# Slackware kernel 2.6.24.5
#flashbin_jffs2_table=`dmesg | tail -n10 | grep block2mtd | grep erase_size | grep "\[d:" | cut -d":" -f2 | cut -b5 | tr '\n' ' '`
#flashbin_mt_devices=`dmesg | tail -n10 | grep block2mtd | grep erase_size | grep "\[d:" | cut -d":" -f2 | cut -d" " -f2 | tr '\n' ' '`

# Debian testing kernel 2.6.25 & 2.6.26
flashbin_mt_devices=`dmesg | tail -n10 | grep block2mtd | grep erase_size | cut -d":" -f2 | cut -d" " -f2 | tr '\n' ' '`
flashbin_jffs2_table=`dmesg | tail -n10 | grep block2mtd | grep erase_size |  cut -d":" -f2 | cut -d" " -f2 | cut -b4 | tr '\n' ' '`

mount_point=`sed -n '/\[partitions\]/,/\[\/partitions\]/{//d;p}' $flashbin_configfile | awk '{ print $2 }' | tr '\n' ' ' `
partitions=`sed -n '/\[partitions\]/,/\[\/partitions\]/{//d;p}' $flashbin_configfile | awk '{ print $1 }' | tr '\n' ' ' `
partitions_type=`sed -n '/\[partitions\]/,/\[\/partitions\]/{//d;p}' $flashbin_configfile | awk '{ print $3 }' | tr '\n' ' ' `
flashbin_rbind_table=`sed -n '/\[rbind_table\]/,/\[\/rbind_table\]/{//d;p}' $flashbin_configfile | awk '{ print $2 }' | tr '\n' ' ' `

dup="dup"
compteur=0

for j in $partitions_type
do
compteur=`expr $compteur + 1`


          flashbin_mountpoint=`echo $flashbin_rbind_table | cut -d" " -f$compteur`
          flashbin_rbind_mountpoint=`echo $mount_point | cut -d" " -f$compteur`
          flashbin_partition=`echo $partitions | cut -d" " -f$compteur`


case "$j" in
     logfs)
          mount --rbind $flashbin_rbind_mountpoint $flashbin_mountpoint$dup 
    	  mount -t logfs $flashbin_partition $flashbin_mountpoint && mount --rbind $flashbin_rbind_mountpoint $flashbin_mountpoint &
     ;;
     jffs2)
          flashbin_mtdblock=`echo $flashbin_jffs2_table | cut -d" " -f $compteur`
	  flashbin_partition="/dev/mtdblock$flashbin_mtdblock"
          mount --rbind $flashbin_rbind_mountpoint $flashbin_mountpoint$dup 
	  mount -t jffs2 $flashbin_partition $flashbin_rbind_mountpoint && mount --rbind $flashbin_rbind_mountpoint $flashbin_mountpoint &
     ;;
     ext3)
          mount --rbind $flashbin_rbind_mountpoint $flashbin_mountpoint$dup
          mount  /dev/$device$flashbin_partition $flashbin_rbind_mountpoint && mount --rbind $flashbin_rbind_mountpoint $flashbin_mountpoint & 
     ;;
esac
done


}

flashbin_configure()
{
dup="dup"
reps=`sed -n '/\[rbind_table\]/,/\[\/rbind_table\]/{//d;p}' $flashbin_configfile | awk '{ print $2 }' | tr '\n' ' ' `
mkdir -p $reps
set $reps
for i
do
mkdir $i$dup
done
return 0
}

flashbin_synchronize()
{

# detecting the key before synchonization
read_serial=`sed -n '/\[serial\]/,/\[\/serial\]/{//d;p}' $flashbin_configfile`
variable=`ls /sys/block | grep sd?*`
set $variable
serial=1

while [ -n "$1" ]
do

  serial=`/usr/bin/udevinfo -a -p /sys/block/$1 | grep -m1 serial | cut -d'=' -f3 | cut -d'"' -f2`

  if  [ "$serial" = "$read_serial" ]
  then
     echo "Key found"
     slashetoile="/*"
     dup="dup"
     flashbin_synchronize_flag=`grep "synchronized" $flashbin_logfile | cut -d"=" -f2`

     if [ $flashbin_synchronize_flag = "yes" ]
     then
        return 0
     elif [ $flashbin_synchronize_flag = "no" ]
     then

        flashbin_synchronize_way=`grep "way" $flashbin_logfile | cut -d"=" -f2`
        flashbin_path_to_synchonize=`sed -n '/\[paths\]/,/\[\/paths\]/{//d;p}' $flashbin_logfile | grep 1 | awk '{ print $1 }' | tr '\n' ' ' `
        echo "Synchronizing $flashbin_path_to_synchonize"

        # synchronisation
        if [ $flashbin_synchronize_way = "flashtodisk" ]
        then
           for i in $flashbin_path_to_synchonize
           do
           		dir_3=`sed -n '/\[rbind_table\]/,/\[\/rbind_table\]/{//d;p}' $flashbin_configfile | grep "$i" | awk '{ print $2 }'`
           		dir_1=$dir_3$slashetoile
           		dir_2=$dir_3$dup
           		echo "cp flashtodisk $dir_1 to $dir_2"
           		cp -ru $dir_1 $dir_2
	   done
        fi

        if  [ $flashbin_synchronize_way = "disktoflash" ]
        then
    	   for i in $flashbin_path_to_synchonize
	       do
	                dir_2=`sed -n '/\[rbind_table\]/,/\[\/rbind_table\]/{//d;p}' $flashbin_configfile | grep "$i" | awk '{ print $2 }'`
           		dir_3=$dir_2$dup
           		dir_1=$dir_3$slashetoile
	                echo "cp disktoflash  $dir_1 to $dir_2"
        		cp -ru $dir_1 $dir_2
           done
        fi
     else
			echo "No rule for this case"

     fi #end of synchonize_flag = yes

    break
  else
     flashbin_device="erreur";
     echo "key not find"
  fi # end of serial detection
     shift 1
done

  if [ -f $synchronisation_run_file ]
  then
    	  rm $synchronisation_run_file
  fi
    # Generation of a new logfile
	killall -SIGUSR1 flashbin  

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
		if [ "$flag_key" = "true" ]
		then
			echo "cle detectee"
			$start_daemon flash &
		else
			echo "cle non detectee"
			$start_daemon &
		fi
     ;;
     synchronize)
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
 
