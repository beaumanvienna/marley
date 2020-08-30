#!/bin/bash


#Ubuntu 20 and derivatives such as Linux Mint 20 need GTK3

PRETTY_NAME=`grep PRETTY_NAME /etc/os-release`
IS_UBUNTU=`echo $PRETTY_NAME | grep Ubuntu`
IS_ARCH=`echo $PRETTY_NAME | grep Arch`
IS_MINT=`echo $PRETTY_NAME | grep Mint`
IS_VERSION_NUMBER_18_04=`echo $PRETTY_NAME | grep 18.04`
IS_VERSION_NUMBER_20_04=`echo $PRETTY_NAME | grep 20.04`
IS_VERSION_NUMBER_20=`echo $PRETTY_NAME | grep 20`

if test -n "$IS_ARCH" 
then
    #echo "Archibald found"
    DUMMY=`echo "null"`
fi

if test -n "$IS_MINT" 
then

    if test -n "$IS_VERSION_NUMBER_20" 
    then
          #echo "Version 20.04 found"
          echo "-DGTK3_API=ON"
    fi
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


