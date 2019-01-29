#!/bin/bash

if [ ! -d "${HOME}/.multiverse/" ]; then
    mkdir -p ${HOME}/.multiverse/
fi

if [ ! -f "${HOME}/.multiverse/multiverse.conf" ]; then
    cp /multiverse.conf ${HOME}/.multiverse/multiverse.conf
fi

exec "$@"