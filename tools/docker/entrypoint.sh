#!/bin/bash

service mysql stop
service mysql start

mysql -e "grant all privileges on *.* to 'multiverse'@'%' identified by '${DB_PASSWORD}';"
mysql -e "flush privileges;"
mysql -e "create database if not exists multiverse;"

tail -f /dev/null