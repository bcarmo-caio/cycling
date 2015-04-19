#!/bin/bash

error=0



echo "testing creating treads"
echo $@
IFS="
"
THREADS_TESTS=10
for i in $(seq $THREADS_TESTS); do
	echo "Testing $i of $THREADS_TESTS"
	DISTINTS_THREAD_IDS=$(./cycling $@ | grep waiting |\
		awk -F" " '{print $4}' | sort | uniq | wc -l)
	if [[ "$DISTINTS_THREAD_IDS" != "$2" ]]; then
		echo "Fail threads id waiting"
		error=1
		break;
	fi
done

if [[ "$error" == "1" ]]; then
	echo "wasted"
else
	echo "threads creation OK"
fi
