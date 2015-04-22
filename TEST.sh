#!/bin/bash
i=1;
while true; do
	./cycling $1 $2 u  > /dev/null 2>&1
	if [[ "$?" != "0" ]]; then
		echo ops $i
		exit 1
	else
		echo foi $i
	fi
	let i++
done

exit 0
