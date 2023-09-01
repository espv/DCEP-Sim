#!/bin/bash
xauth list > out
XAUTH_COOKIE=$(echo $DISPLAY | grep -oE ":[[:digit:]]+" | grep -f - out | grep -oE "\w{32}")
rm out

if [ -z "$XAUTH_COOKIE" ]
then
	XAUTH_COOKIE=$(xauth list | grep "" | head -1 | grep -oE "\w{32}")
fi

echo $XAUTH_COOKIE
