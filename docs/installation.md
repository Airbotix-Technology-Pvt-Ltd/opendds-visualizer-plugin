# OpenDDS Visualizer Plugin — Installation and Usage Guide

The **OpenDDS Visualizer Plugin** integrates with **PlotJuggler** to visualize DDS topic data in real time.
This guide provides two installation methods: **Docker-based** (recommended) and **manual build from source**.

---

## 1. Docker-Based Installation (Recommended)

This method uses a Docker container to automatically handle all dependencies, environment variables, and build steps.

### Steps

1. **Download and source the setup script**:

```bash
wget -O setup.sh https://raw.githubusercontent.com/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin/opendds/setup.sh
source ./setup.sh
```

2. **Pull the latest PlotJuggler Docker setup**:

```bash
plotjuggler pull
```

3. **Build the Docker image**:

```bash
plotjuggler build
```

4. **Start the container**:

```bash
plotjuggler start
```

5. **Launch PlotJuggler inside the container**:

```bash
plotjuggler
```

> The container includes all required Qt packages, OpenDDS libraries, and the OpenDDS Visualizer Plugin.

---

## 2. Manual Build from Source

This method installs dependencies directly on your host and builds the plugin from source. Steps follow the Dockerfile as reference.

### 2.1 Install Build Tools

```bash
sudo apt update
sudo apt install -y g++ cmake python3-pip wget git
pip3 install --upgrade colcon-common-extensions vcstool
```

> **Note on compilers:** While the instructions use `g++`, this project can also be built with `clang`.

### 2.2 Install Qt Packages

```bash
sudo apt install -y qtbase5-dev libqt5websockets5-dev libqt5x11extras5-dev libqt5svg5-dev
```

### 2.3 Install Additional Libraries

```bash
sudo apt install -y libasio-dev libtinyxml2-dev libssl-dev
```

### 2.4 Install OpenDDS

> Before building the plugin, you need to have OpenDDS installed on your system. You can find the installation instructions in the [OpenDDS Developer's Guide](https://opendds.readthedocs.io/en/latest/getting_started/index.html).

### 2.5 Clone Plugin Repository and Pull Dependencies

```bash
git clone --recursive https://github.com/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin.git
cd opendds-visualizer-plugin
wget https://raw.githubusercontent.com/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin/opendds/opendds_visualizer_plugin.repos
vcs import --recursive src < opendds_visualizer_plugin.repos
```

> `vcs` automatically pulls PlotJuggler and other dependencies.

### 2.6 Build the Plugin

```bash
CC=/usr/bin/gcc-11 CXX=/usr/bin/g++-11 colcon build --cmake-args
```

### 2.7 Configure Environment Variables

```bash
source install/setup.bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:install/lib:/opt/OpenDDS/lib:/opt/OpenDDS/ACE_wrappers/lib
export PATH=$PATH:install/bin
export DDS_CONFIG_FILE=/usr/local/share/dds/rtps.ini
```

> Add these lines to `~/.bashrc` for persistence.

### 2.8 Create RTPS Configuration File (if you have not any)

The `rtps.ini` file is used by OpenDDS to configure the transport protocol. The following command creates a basic configuration file that uses the RTPS protocol over UDP.

```bash
sudo mkdir -p /usr/local/share/dds
cat << 'EOF' | sudo tee /usr/local/share/dds/rtps.ini > /dev/null
[common]
DCPSGlobalTransportConfig=$file
DCPSDefaultDiscovery=DEFAULT_RTPS
DCPSPendingTimeout=30

[transport/the_rtps_transport]
transport_type=rtps_udp
EOF
```

### 2.9 Run PlotJuggler with the Plugin

```bash
plotjuggler -n --plugin_folders install/opendds_visualizer_plugin/bin/
```

1. Go to **DataStreamer → Load DataStreamer** and select **OpenDDS Visualizer Plugin**.
2. Enter the DDS Domain ID and click **Connect**.
3. Select the topics to visualize and click **Start**.

---