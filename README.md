# OpenDDS Visualizer Plugin for PlotJuggler

The *OpenDDS Visualizer Plugin* is a specialized plugin designed for the *PlotJuggler* application. PlotJuggler is a powerful graphical desktop application that provides comprehensive visualization capabilities for data series, time series, and X-Y plots. Beyond visualization, it offers robust data management features including data import and export functionality, custom and built-in data manipulation functions, and data series merging capabilities. Additionally, the software supports multiple different layouts with dynamic, rich, and user-friendly customization options.

The *OpenDDS Visualizer Plugin* enables users to visualize topic-related data directly from a DDS network. Users can select multiple topics from the discovered topics currently running in the DDS network. These selected topics are automatically divided by their values using data type introspection technology, allowing each individual value to be visualized and managed separately. This functionality empowers users to quickly visualize the detailed data content being exchanged across the network in various different ways.

## Key Features

The *OpenDDS Visualizer Plugin* provides the following comprehensive features:

1. **Data Type Introspection**: By leveraging *OpenDDS Dynamic XTypes*, this plugin enables automatic discovery of topic data types and visualization of data content using the corresponding appropriate data type structure.

2. **DDS Configurations**: Multiple different configurations can be established to specify the *Domain Id* and to select specific *DDS Topics* for visualization purposes.

3. **Complete PlotJuggler Feature Integration**: This plugin seamlessly integrates with all PlotJuggler features, enabling users to create rich, sophisticated graphs from advanced data manipulations.

## Commercial Support

Looking for commercial support? Contact us at shubham.garg@airbotix.in

Learn more about our services at [airbotix's webpage](https://airbotix.in/).

## Documentation

Documentation is currently in development (To Do).

### Reference Documentation

* [Installation Manual](https://plotjuggler-fastdds-plugins.readthedocs.io/en/latest/rst/installation/linux.html)
* [Getting Started Guide](https://plotjuggler-fastdds-plugins.readthedocs.io/en/latest/rst/getting_started/tutorial.html)
* [User Manual](https://plotjuggler-fastdds-plugins.readthedocs.io/en/latest/rst/user_manual/start_plugin.html)
* [Developer Manual](https://plotjuggler-fastdds-plugins.readthedocs.io/en/latest/rst/developer_manual/installation/sources/linux.html)
* [Release Notes](https://plotjuggler-fastdds-plugins.readthedocs.io/en/latest/rst/notes/notes.html)

## Credits

This plugin has been adapted from the [fastdds-visualizer-plugin](https://github.com/eProsima/fastdds-visualizer-plugin) and the OpenDDS Monitor project at https://github.com/OpenDDS/opendds-monitor

---

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