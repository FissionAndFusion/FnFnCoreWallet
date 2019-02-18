#!/bin/bash

cp ${APPS_DIR}/multiverse /usr/local/bin/
cp ${APPS_DIR}/multiverse-cli /usr/local/bin/
cp ${APPS_DIR}/multiverse-dnseed /usr/local/bin/
cp ${APPS_DIR}/multiverse-miner /usr/local/bin/
cp ${APPS_DIR}/multiverse-server /usr/local/bin/
chmod 755 /usr/local/bin/multiverse

mysql -u root -P ${DB_PORT} -h ${DB_HOST} -e "grant all privileges on *.* to 'multiverse'@'%' identified by 'multiverse';"
mysql -u root -P ${DB_PORT} -h ${DB_HOST} -e "flush privileges;"
mysql -u root -P ${DB_PORT} -h ${DB_HOST} -e "create database if not exists ${DB_NAME};"

if [ -f "${HOME}/.multiverse/multiverse.conf" ]; then
    rm ${HOME}/.multiverse/multiverse.conf
fi

if [ ! -d "${HOME}/.multiverse/" ]; then
    mkdir ${HOME}/.multiverse/
fi

touch ${HOME}/.multiverse/multiverse.conf

if [ -n "${DNSEED}" ]; then
    echo "addnode=${DNSEED}" >> ${HOME}/.multiverse/multiverse.conf
fi
if [ -n "${DNSEED_PORT}" ]; then
    echo "port=${DNSEED_PORT}" >> ${HOME}/.multiverse/multiverse.conf
fi

echo "dbhost=${DB_HOST}" >> ${HOME}/.multiverse/multiverse.conf
echo "dbport=${DB_PORT}" >> ${HOME}/.multiverse/multiverse.conf
echo "dbname=${DB_NAME}" >> ${HOME}/.multiverse/multiverse.conf

if [ -n "${DBP_ALLOW_IP}" ]; then
    echo "dbpallowip=${DBP_ALLOW_IP}" >> ${HOME}/.multiverse/multiverse.conf
fi

if [ -n "${DBP_R_HOST}" ]; then
    echo "dbpparentnodeip=`host ${DBP_R_HOST} | awk '/has address/ { print $4 }'`" >> ${HOME}/.multiverse/multiverse.conf
fi

# host db | awk '/has address/ { print $4 }'

# if [ -n "${DBP_R_PORT}" ]; then
#     echo "dbpparentnodeport=${DBP_R_PORT}" >> ${HOME}/.multiverse/multiverse.conf
# fi

if [ -n "${FORK_ID}" ]; then
    echo "addfork=${FORK_ID}" >> ${HOME}/.multiverse/multiverse.conf
fi

if [ -n "${GROUP_ID}" ]; then
    echo "addgroup=${GROUP_ID}" >> ${HOME}/.multiverse/multiverse.conf
fi

echo "enablesupernode=${ENABLE_SN}" >> ${HOME}/.multiverse/multiverse.conf
echo "enableforknode=${ENABLE_FN}" >> ${HOME}/.multiverse/multiverse.conf

multiverse -debug