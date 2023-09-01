#!/bin/sh
touch /root/.Xauthority
xauth add $DISPLAY . $XAUTH_COOKIE
