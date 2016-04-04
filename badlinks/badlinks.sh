#!/bin/bash

E_WRONG_ARGS=85
WEEK_IN_SECS=$((60*60*24*7))

if [ $# -ne 1 ]
then
	echo "Usage: `basename "$0"` <path>"
	exit $E_WRONGARGS
fi

links=`find -L "$1"`
#echo `date +%s`
for link in $links
do
	age=$(( $(date +%s) -  $(stat -c %Y "$link") ))
	if [ $age -ge $WEEK_IN_SECS ] && [ ! -e "$link" ]
	then
		echo $link		
	fi
done

