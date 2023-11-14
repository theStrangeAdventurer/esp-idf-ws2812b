#!/bin/sh

if [ "$1" == "--flash" ]; then
	idf.py build && idf.py -p /dev/tty.usbserial-0001 flash monitor
elif [ "$1" == "--clear" ]; then
	rm -rf ./build && idf.py build
elif [ "$1" != "--flash" ]; then
	idf.py build
fi
