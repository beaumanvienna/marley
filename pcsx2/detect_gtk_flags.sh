#!/bin/bash


#Ubuntu 20 and derivatives such as Linux Mint 20 need GTK3

PCSX2_USE_GTK3=`find /usr/ -name "*wx_gtk3u_core-3.0*.so" 2>/dev/null`

if [ -z "$PCSX2_USE_GTK3" ]
then
    DUMMY=`echo "null"`
else
    echo "-DGTK3_API=ON"
fi
