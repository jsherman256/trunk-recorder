#!/bin/bash

/usr/bin/docker run -d \
	--name jsherman256-trunk-recorder \
	-e TZ=America/Los_Angeles \
	-v /home/pi/jsherman256-trunk-recorder:/app \
	--device /dev/bus/usb \
	jsherman256-trunk-recorder
