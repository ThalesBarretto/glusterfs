#!/usr/bin/env bash

vagrant up --no-provision $@
vagrant provision
