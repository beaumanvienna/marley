#!/bin/bash

PRETTY_NAME=`grep PRETTY_NAME /etc/os-release`
IS_UBUNTU=`echo $PRETTY_NAME | grep Ubuntu`
IS_ARCH=`echo $PRETTY_NAME | grep Arch`
IS_VERSION_NUMBER_18_04=`echo $PRETTY_NAME | grep 18.04`
IS_VERSION_NUMBER_20_04=`echo $PRETTY_NAME | grep 20.04`

if test -n "$IS_ARCH" 
then
    #echo "Archibald found"
    DUMMY=`echo "null"`
fi

if test -n "$IS_UBUNTU" 
then
    #echo "Ubuntu found"
    if test -n "$IS_VERSION_NUMBER_18_04" 
    then
        #echo "Version 18.04 found"
        DUMMY=`echo "null"`
    fi

    if test -n "$IS_VERSION_NUMBER_20_04" 
    then
          #echo "Version 20.04 found"
          echo "-DGTK3_API=ON"
    fi
fi


