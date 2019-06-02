#!/usr/bin/env bash

#SIK HELPERS

export PATH=${PATH}:${SIKBLD}/src
export MULTI_ADDR=239.13.41.5
export MULTI_PORT=3000
export SPACE=1000000000000000

C1_CL=(
    -g 239.13.41.15
    -p 3000
    -t 1
)
C2_CL=(
    -g 159.65.124.229
    -p 3000
    -t 3
)
C1=(
    -g 239.13.41.15
    -p 3000
    -t 3
)
C2=(
    -g 159.65.124.229
    -p 3000
    -t 3
)

alias valgrind="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes"

function sikbuild(){
	curr=$(pwd)
	cd ${SIKBLD} && make -j8
	cd $curr
}


# Example usage
# source tests.sh
# mkdir test
# cd test
# stress_upload 50 239.13.41.15 3000
# when done kill_clients
function kill_clients(){
    exec 3>&-
}

function stress_upload(){
    cd ~/test/
    rm -rf client_*
    CLIENTS=$1
    IP=$2
    PORT=$3
    VALGRIND=$4
    echo "print('A' * 100000000)" | python > "_upload"
    for i in $(seq 1 ${CLIENTS}); do
        echo "Spawning client ${i}"
        OUTDIR="client_${i}"
        rm -rf ${OUTDIR}
        mkdir ${OUTDIR}
        UPFILE="${OUTDIR}/upload_${i}"
        cp "_upload" ${UPFILE}
        FIFO="${OUTDIR}/fifo"
        rm -rf ${FIFO}
        mkfifo ${FIFO}
        ${VALGRIND} client -g ${IP} -p ${PORT} -o ${OUTDIR} -t 1 < "${FIFO}" > "${OUTDIR}/log" &
        exec 3>${FIFO}
        printf "discover\nsearch\nupload ${UPFILE}\n" > ${FIFO}
    done;
    rm -rf "_upload"
}

function generate_shared_dir() {
    DIR=$1
    COUNT=$2
    cd ${DIR}
    rm -rf *
    for i in $(seq 1 ${COUNT}); do
        FILE=$(printf "import uuid\nprint(uuid.uuid4())\n" | python)
        echo "0123456789" > ${FILE}
    done;
}


function start_test_server(){
    generate_shared_dir ~/test/serverdir 10 && sikbuild && $1 server "${C1[@]}" -f ~/test/serverdir -b ${SPACE}
}