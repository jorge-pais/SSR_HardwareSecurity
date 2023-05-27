#!/bin/bash

if [ -z "$1" ]; then
  echo "Usage: $0 <new_value>"
  exit 1
fi

cacheT="$1"
sed -i "s/#define CACHE_THRESHOLD .*/#define CACHE_THRESHOLD $cacheT/" attack/attack.c