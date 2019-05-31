#!/usr/bin/env bash

#SIK HELPERS

export PATH=${PATH}:${SIKBLD}/src

alias valgrind="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes"

function sikbuild(){
	curr=$(pwd)
	cd ${SIKBLD} && make -j8
	cd $curr
}

function spawnservers(){
	for i in $(seq 1 $1); do server $SIKIP $SIKPORT & done
}

