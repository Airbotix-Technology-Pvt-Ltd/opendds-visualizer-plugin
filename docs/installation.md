
# OpenDDS Visualizer Plugin - Installation and Usage Guide

## Prerequisites and Setup

When working in a minimal container environment, before installing `qt5-default` and `libqt5websockets5-dev`, you must first add the universe repository, as many Qt development packages are hosted there. This is why `software-properties-common` is required as the initial step.

### System Setup Instructions

Execute the following commands in sequence:

```shell
apt update
apt install -y software-properties-common
add-apt-repository universe
apt update
```

### Install Qt Packages

After adding the universe repository, install the required Qt packages:

```shell
apt install -y qtbase5-dev libqt5websockets5-dev libqt5x11extras5-dev
```

### Install Additional Dependencies

Install ASIO and TinyXML2 libraries:

```shell
apt install libasio-dev libtinyxml2-dev
```

### Required Qt Packages Summary

To recap, the following Qt packages are essential for your container:

* **qtbase5-dev** → Core Qt framework
* **libqt5websockets5-dev** → WebSockets support
* **libqt5x11extras5-dev** → X11 integration

## Building the Plugin

### Clone the plugin
```
git clone --recursive git@github.com:Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin.git
```
### Install Build Tools

First, install the required GCC compiler version:

```shell
apt install g++-11
```

### Build Process

Build the plugin using colcon with specific compiler versions:

```shell
CC=/usr/bin/gcc-11 CXX=/usr/bin/g++-11 colcon build --cmake-args
```

## Running PlotJuggler with OpenDDS Plugin

The `OpenDDS Visualizer Plugin` for PlotJuggler enables real-time visualization of data published on an OpenDDS network. It provides a seamless way to inspect and plot numeric and string data from your DDS topics.

### Usage

1.  **Launch PlotJuggler:**
    ```bash
    plotjuggler
    ```
    If the build was completed using colcon, ensure you source all projects first:
    ```bash
    source install/setup.bash
    ```
    If the build was performed using CMake directly, extend the following environment variables with your installation `lib/` and `bin/` paths:
    ```bash
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/install/lib
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/OpenDDS/lib:/opt/OpenDDS/ACE_wrappers/lib
    export PATH=$PATH:~/install/bin
    ```
2.  **Load the Plugin:** In PlotJuggler, navigate to `DataStreamer` -> `Load DataStreamer` and select `OpenDDS Visualizer Plugin`.
3.  **Connect to DDS Domain:** A dialog will prompt you to enter the DDS Domain ID. Provide the correct ID and click `Connect`.
4.  **Select Topics:** Another dialog will display a list of discovered DDS topics. Select the topics whose data you want to visualize and click `Start`.

The plugin will then begin streaming the data from your chosen DDS topics, and you can configure PlotJuggler to display the desired fields.