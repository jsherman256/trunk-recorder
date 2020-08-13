#!/bin/bash

/usr/bin/docker run -d --rm \
	--name tr \
	-e TZ=America/Los_Angeles \
	-v /home/pi/jsherman256-trunk-recorder/app:/app \
	--device /dev/bus/usb \
	jsherman256/trunk-recorder
