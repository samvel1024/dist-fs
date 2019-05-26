#!/usr/bin/env bash

#SIK HELPERS

export PATH=${PATH}:${SIKBLD}/src

function sikbuild(){
	curr=$(pwd)
	cd ${SIKBLD} && make
	cd $curr
}

function spawnservers(){
	for i in $(seq 1 $1); do server $SIKIP $SIKPORT & done
}

