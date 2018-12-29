#!/bin/bash

if [ ! -d "${HOME}/.multiverse/" ]; then
    mkdir -p ${HOME}/.multiverse/
fi

if [ ! -f "${HOME}/.multiverse/multiverse.conf" ]; then
    cp /multiverse.conf ${HOME}/.multiverse/multiverse.conf
fi

chown -R mysql:mysql /var/lib/mysql
mysqld --initialize-insecure --user=mysql
service mysql stop
service mysql start

mysql -e "grant all privileges on *.* to 'multiverse'@'%' identified by '${DB_PASSWORD}';"
mysql -e "flush privileges;"
mysql -e "create database if not exists multiverse;"

#tail -f /dev/null
exec "$@"