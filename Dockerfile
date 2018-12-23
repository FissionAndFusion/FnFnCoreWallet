FROM ubuntu:16.04
# MAINTAINER FnFn

ENV DEBIAN_FRONTEND noninteractive
ENV HOME=/home/fnfn
ENV DB_PASSWORD=multiverse

RUN apt-get update \
  && apt-get -yq install mysql-server \
  && apt-get install -yq libssl-dev \
  && apt-get install -yq libmysqlclient-dev \
  && apt-get install -yq libsodium-dev \
  && apt-get install -yq libreadline6-dev \
  && rm -rf /var/lib/apt/lists/* \
  && usermod -d /var/lib/mysql/ mysql \
  && find /etc/mysql/ -name '*.cnf' -print0 \
  | xargs -0 grep -lZE '^(bind-address|log)' \
  | xargs -rt -0 sed -Ei 's/^(bind-address|log)/#&/' \
  && echo '[mysqld]\nskip-host-cache\nskip-name-resolve' > /etc/mysql/conf.d/docker.cnf

COPY entrypoint.sh /sbin/
RUN mkdir -p ${HOME}/.multiverse/ && chmod 755 /sbin/entrypoint.sh
COPY build/src/multiverse* /usr/bin/
COPY multiverse.conf ${Home}/.multiverse/
VOLUME ["${HOME}", "/etc/mysql/conf.d", "/var/lib/mysql" ]
EXPOSE 3306 33060 6811 6812 6815
ENTRYPOINT ["/sbin/entrypoint.sh"]
CMD ["mysqld_safe"]
