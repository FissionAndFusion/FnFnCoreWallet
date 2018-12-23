FROM mysql:5.7
MAINTAINER gaochun@fnfn.io

ENV HOME=/home/fnfn

ENV DB_HOST=localhost
ENV DB_PORT=3306
ENV DB_NAME=multiverse

RUN apt-get update \
    && apt-get upgrade -y \
    && apt-get install -y libssl1.0-dev \
    # && apt-get install -y libssl1.1 \
    && apt-get install -y libmysqlclient-dev \
    && apt-get install -y libsodium-dev \
    # && apt-get install -y libreadline6-dev \
    && apt-get install -y libreadline7 \
    && apt-get install -y mysql-client

COPY build/src/multiverse* /usr/local/bin/
COPY entrypoint.sh /sbin/

RUN mkdir /home/fnfn \
    && chmod 755 /sbin/entrypoint.sh 

VOLUME ["${HOME}"]
EXPOSE 6815 6812
ENTRYPOINT ["/sbin/entrypoint.sh"]