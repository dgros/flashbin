#!/bin/bash

flashbin_log="/home/utilisateur/workspace/flashbin_kiki/flashbin/flashbin.log"


if [ "$1" = "/usr/bin" ]
	then
		if [ ! -f "fichier_bin.txt" ]
			then
				`ls -Rl $1 > fichier_bin.txt`
		fi
value=`grep "$1" $flashbin_log | cut -d"=" -f2`
	
		if [ $value = 0 ]
			then
				`ls -Rl $1 > fichier_test_b.txt`

				variable=`diff fichier_bin.txt fichier_test_b.txt`
	
				if [ -n "$variable" ]
					then
						touch $1/fichier_modifier
						rm $1/fichier_modifier
				fi
				rm fichier_test*
		fi
fi



if [ "$1" = "/usr/lib" ]
	then
		if [ ! -f "fichier_lib.txt" ]
			then
				`ls -Rl $1 > fichier_lib.txt`
		fi

value=`grep "$1" $flashbin_log | cut -d"=" -f2`
		if [ $value = 0 ]
			then
				`ls -Rl $1 > fichier_test_l.txt`

				variable=`diff fichier_lib.txt fichier_test_l.txt`

				if [ -n "$variable" ]
					then
						touch $1/fichier_modifier
						rm $1/fichier_modifier
				fi
				rm fichier_test*
		fi
fi



if [ "$1" = "/var" ]
	then
		if [ ! -f "fichier_var.txt" ]
			then
				`ls -Rl $1 > fichier_var.txt`
		fi
value=`grep "$1" $flashbin_log | cut -d"=" -f2`
		if [ $value = 0 ]
			then
				`ls -Rl $1 > fichier_test_v.txt`

				variable=`diff fichier_var.txt fichier_test_v.txt`

				if [ -n "$variable" ]
					then
						touch $1/fichier_modifier
						rm $1/fichier_modifier
				fi
				rm fichier_test*
		fi
fi

