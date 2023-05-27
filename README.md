# System Security - Spectre and Meltdown Hardware Vulnerabilities

This repository holds both the main project contribution, a Spectre V1 Proof-of-Concept for reading kernel space memory, as well as the solutions for the System Security SEED Labs on the Meltdown and Spectre hardware vulnerabilities. These were done for the final project for FEUP's Systems and Network Security course of the MsC in Electrical and Computer Engineering.

## Spectre Variant 1 PoC

This attack consists of two parts: a vulnerable kernel module and a attacking program which attempts to read Kernel memory from user space.

To build both the attack and the loadable kernel module:

``` bash
cd ./src
make -C attack/
make -C module/
```

To load/unload the module and check if it is working correctly:
``` bash
# Load 
sudo insmod module/spectreModule.ko
# Unload
sudo rmmod spectreModule

# Check the kernel messages
tail /var/log/syslog 
dmesg
# Check the secret memory address
cat /proc/leakTheAddress
```

The cache threshold must be set accordingly in order for the attack to work. To measure the memory hierarchy access times and set the values:

```bash
# Compile and run the measurements
gcc -o cache CacheTime.c && ./cache

chmod +x changeCacheThre.sh
./changeCacheThre.sh 1337 # Change this value accordingly
```

Then build and run the `./attack/attack` program to see the results.

### Testing Conditions

This attack was tested utilizing a first-gen Intel Core i3-380M CPU, running on Ubuntu 16.04 without any kernel mitigation against either Spectre or Meltdown. 

## SEED Labs

The solutions for the both SEED Labs are present in the subdirectories within `./SEEDLabs/`. These include the solved code examples and the answers to the questions of the tasks for the two labs. For further information, consult the SEED Lab on [Spectre](https://seedsecuritylabs.org/Labs_20.04/System/Spectre_Attack/) and [Meltdown](https://seedsecuritylabs.org/Labs_20.04/System/Meltdown_Attack/) .

## License

All the code is licensed under the GPLv3 public license.