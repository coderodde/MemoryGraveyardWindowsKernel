This device driver was developed under Windows 11 Home version 24H2 (OS Build 26100.4351).

I used the following tools:
1. Visual Studio 2022 Community
2. Windows Driver Kit - Windows 10.0.26100.3323 (may be installed via Visual Studio Installer)
3. MSVC v143 - VS 2022 C++ x64/x86 Spectre-mitigated libs (v14..44-17.14) (may be installed via Visual Studio Installer

## What is it good for?

The driver manages a kilobyte of RAM memory which may be read by user processes and written also.
Basically, it mimics `/dev/random` form *nix systems, yet is not quite suitable for its purpose.

## Installing and running

In order to intall the driver without signing it, you must run
```
bcdedit /set testsigning on
```
The `bcdedit` command requires **a reboot** in order to take effect.

Next, after a reboot, we need to install the driver. The driver `MemoryGraveyardDriver.sys` is located (from the root of this repository, `<SOLUTION_ROOT>`) at `\x64\Release\MemoryGraveyardDriver\`. To this end, open an elevated command line and type:
```
sc create graveyard type= kernel binPath= <SOLUTION_ROOT>\x64\Release\MemoryGraveyardDriver\MemoryGraveyardDriver.sys
```
After creating a service with the above program, you need to start the service:
```
sc start graveyard
```
In order to stop the service, type:
```
sc stop graveyard
```
After stopping the service, you can delete it completely via command:
```
sc delete graveyard
```
