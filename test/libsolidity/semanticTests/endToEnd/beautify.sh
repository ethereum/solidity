#!/bin/bash

for a in `ls *.sol`
do
echo "" >/tmp/ttt.sol
cat $a >> /tmp/ttt.sol
echo "" >>/tmp/ttt.sol
js-beautify -f /tmp/ttt.sol > /tmp/$a
echo "" >>/tmp/$a
cp /tmp/$a $a
done
