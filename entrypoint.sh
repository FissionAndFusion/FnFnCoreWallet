#!/bin/bash

service mysql stop
service mysql start
mysql -u root -P ${DB_PORT} -h ${DB_HOST} -e "grant all privileges on *.* to 'multiverse'@'%' identified by 'multiverse';"
mysql -u root -P ${DB_PORT} -h ${DB_HOST} -e "flush privileges;"
mysql -u root -P ${DB_PORT} -h ${DB_HOST} -e "create database if not exists ${DB_NAME};"

tail -f /dev/null