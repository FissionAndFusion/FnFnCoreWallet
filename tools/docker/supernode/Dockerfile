FROM ubuntu:18.04
# MAINTAINER gaochun@fnfn.io

ENV HOME=/home/fnfn
ENV APPS_DIR=/home/apps

ENV DNSEED=
ENV DNSEED_PORT=

ENV DB_HOST=localhost
ENV DB_PORT=3306
ENV DB_NAME=multiverse

ENV DBP_R_HOST=
ENV DBP_R_PORT=6815
ENV FORK_ID=

RUN apt-get update \
&& apt-get upgrade -y \
&& apt-get install -y libssl1.0-dev \
&& apt-get install -y libssl1.1 \
#&& apt-get install -y libboost-all-dev \
&& apt-get install -y libmysqlclient-dev \
&& apt-get install -y libsodium-dev \
&& apt-get install -y libreadline6-dev \
&& apt-get install -y libreadline7 \
&& apt-get install -y mysql-client
#&& apt-get install -y iproute2

#COPY multiverse /usr/local/bin/
COPY entrypoint.sh /sbin/

RUN mkdir /home/fnfn \
&& mkdir /home/apps \
#&& chmod 755 /usr/local/bin/multiverse \
&& chmod 755 /sbin/entrypoint.sh 
#&& ip -4 route list match 0/0 | awk '{print $3 "host.docker.internal"}' >> /etc/hosts

VOLUME ["${HOME}", "${APPS_DIR}"]
EXPOSE 6815 6812
ENTRYPOINT ["/sbin/entrypoint.sh"]
