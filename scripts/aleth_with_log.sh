#!/usr/bin/env bash

ALETH_PATH=$1
ALETH_TMP_OUT=$2
shift
shift


$ALETH_PATH $@ &> >(tail -n 100000 &> "$ALETH_TMP_OUT") &

PID=$!

function cleanup()
{
	kill $PID
}

trap cleanup INT TERM

wait $PID

