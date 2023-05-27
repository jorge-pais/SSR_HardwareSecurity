#!/bin/bash

# This script is used to enable/disable kernel mitigations for spectre
# To check what mitigations are currently enabled for your kernel
# use the https://github.com/speed47/spectre-meltdown-checker shell script

if grep -q "nospectre_v1" /etc/default/grub && grep -q "nospectre_v2" /etc/default/grub; then
  sed -i 's/ nospectre_v1 nospectre_v2//' /etc/default/grub
  update-grub
  echo "Flags removed"
else
  if grep -q "GRUB_CMDLINE_LINUX_DEFAULT" /etc/default/grub; then
    sed -i 's/GRUB_CMDLINE_LINUX_DEFAULT="/&nospectre_v1 nospectre_v2 /' /etc/default/grub
  else
    echo "GRUB_CMDLINE_LINUX_DEFAULT=\"nospectre_v1 nospectre_v2\"" >> /etc/default/grub
  fi
  update-grub
  echo "Flags added"
fi