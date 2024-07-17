# IoPerf

A Windows kernel driver that measures how much time processes spend on IO. 

This repository contains 2 VS projects: `IoPerf` is the minifilter kernel driver,
and `UMapp` is the user-mode app to interface with the driver.

## How it Works

IoPerf captures IRPs (Io Request Packets) between processes and the IO Manager.
For each IRP (and Fast-IO), IoPerf measures the time the packet took to be processed.
The user-mode app connects to the driver via a `flt` port and accumulates the measurements.

The times recorded is the accumulated time for every IRP a process has sent. 
Therefore it's possible that the time recorded exceeds the time a process has existed for.
In such a scenario, there are probably multiple threads executing IO processes, or IO is happening asyncronously.

## Installation

Note: only x64 architecture is supported.

Clone the repo and build *both* projects. `IoPerf` depends on WDK and a Windows SDK.

Install the kernel driver by right-clicking the `.inf` file and select the `Install` option.

Use `sc start IoPerf` to start the driver, and `fltmc attach IoPerf <volume>` to attach it to a volume.

Run `IoPerf.exe` as administrator to connect to the port.

## Bugs

- Some processes, notably `Windows Search Indexer`, generate weird IRPs that don't make sense. This is being looked into.
- Transcational IRPs are currently not supported

## Todo

- Better user interface
- Additional data and view, like `gperftools` (Use sql database with JS parser)
- Ability to pause, start, stop, and switch volumes from CLI
- Save data to file
