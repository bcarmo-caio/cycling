#!/bin/bash

error=0

echo "testing creating treads"
IFS="
"
THREADS=300
THREADS_TESTS=10
for i in $(seq $THREADS_TESTS); do
	echo "Testing $i of $THREADS_TESTS"
	DISTINTS_THREAD_IDS=$(./cycling 1 $THREADS v | grep waiting |\
		awk -F" " '{print $4}' | sort | uniq | wc -l)
	if [[ "$DISTINTS_THREAD_IDS" != "$THREADS" ]]; then
		echo "Fail threads id waiting"
		error=1
		break;
	fi
	DISTINTS_THREAD_GONE=$(./cycling 1 $THREADS v | grep gone |\
		awk -F" " '{print $4}' | sort | uniq | wc -l)
	if [[ "$DISTINTS_THREAD_GONE" != "$THREADS" ]]; then
		echo "Fail threads id gone"
		error=1
		break
	fi
	sleep 1
done

if [[ "$error" == "1" ]]; then
	echo "wasted"
else
	echo "threads creation OK"
fi
