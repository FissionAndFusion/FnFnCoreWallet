FROM ubuntu:16.04
# MAINTAINER FnFn

ENV DEBIAN_FRONTEND noninteractive
ENV HOME=/home/fnfn
ENV DB_PASSWORD=multiverse

RUN groupadd -r mysql && useradd -r -g mysql mysql
RUN apt-get update \
  && apt-get install -yq mysql-server --no-install-recommends \
  && apt-get install -yq libssl-dev --no-install-recommends \
  && apt-get install -yq libmysqlclient-dev --no-install-recommends \
  && apt-get install -yq libsodium-dev --no-install-recommends \
  && apt-get install -yq libreadline6-dev --no-install-recommends \
  && rm -rf /var/lib/apt/lists/* \
  && rm -rf /var/lib/mysql && mkdir -p /var/lib/mysql /var/run/mysqld \
	&& chown -R mysql:mysql /var/lib/mysql /var/run/mysqld \
  && chmod 777 /var/run/mysqld \
  && find /etc/mysql/ -name '*.cnf' -print0 \
  | xargs -0 grep -lZE '^(bind-address|log)' \
  | xargs -rt -0 sed -Ei 's/^(bind-address|log)/#&/' \
  && echo '[mysqld]\nskip-host-cache\nskip-name-resolve' > /etc/mysql/conf.d/docker.cnf

VOLUME ["${HOME}", "/var/lib/mysql" ]
COPY entrypoint.sh /sbin/
RUN chmod 755 /sbin/entrypoint.sh
COPY build/multiverse* /usr/bin/
COPY multiverse.conf /multiverse.conf
EXPOSE 3306 33060 6811 6812 6815
ENTRYPOINT ["/sbin/entrypoint.sh"]
CMD ["multiverse"]
