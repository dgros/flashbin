#!/bin/bash


if [ "$1" = "/usr/bin" ]
	then
		if [ ! -f "fichier_bin.txt" ]
			then
				`ls -Rl $1 > fichier_bin.txt`
		fi

		`ls -Rl $1 > fichier_test.txt`

variable=`diff fichier_bin.txt fichier_test.txt`
	
	if [ -n "$variable" ]
		then
			touch $1/fichier_modifier
			rm $1/fichier_modifier
	fi
fi

if [ "$1" = "/usr/lib" ]
	then
		if [ ! -f "fichier_lib.txt" ]
			then
				`ls -Rl $1 > fichier_lib.txt`
		fi

		`ls -Rl $1 > fichier_test.txt`

variable=`diff fichier_lib.txt fichier_test.txt`

	if [ -n "$variable" ]
		then
			touch $1/fichier_modifier
			rm $1/fichier_modifier
	fi
fi

if [ "$1" = "/var" ]
	then
		if [ ! -f "fichier_var.txt" ]
			then
				`ls -Rl $1 > fichier_var.txt`
		fi

		`ls -Rl $1 > fichier_test.txt`

variable=`diff fichier_var.txt fichier_test.txt`

	if [ -n "$variable" ]
		then
			touch $1/fichier_modifier
			rm $1/fichier_modifier
	fi
fi

rm $1/fichier_modifier
