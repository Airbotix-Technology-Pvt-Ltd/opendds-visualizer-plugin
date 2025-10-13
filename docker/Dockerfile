FROM ghcr.io/opendds/opendds:latest-release
WORKDIR /DDS-Visualizer-Plugin/src

# Install required tools
RUN apt-get update && apt-get install -y \
    curl \
    gnupg \
    lsb-release \
    ca-certificates \
    software-properties-common \
    gnupg2 \
    build-essential \
    openssh-client 

# Install dependencies for opendds-visualizer-plugin
RUN apt-get update && apt-get install -y \  
    # Build tools
    cmake \
    g++ \
    pip \
    wget \
    git \
    # Install Qt5 and dependencies
    qtbase5-dev \
    libqt5websockets5-dev \
    libqt5x11extras5-dev \
    # Asio and TinyXML2 libraries
    libasio-dev \
    libtinyxml2-dev \
    # OpenSSL
    libssl-dev && \
    rm -rf /var/lib/apt/lists/*

# Install colcon and vcstool
RUN pip3 install -U \
    colcon-common-extensions \
    vcstool 

# Set workspace root for build
WORKDIR /DDS-Visualizer-Plugin

RUN wget https://raw.githubusercontent.com/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin/main/opendds_visualizer_plugin.repos && \
    vcs import src < opendds_visualizer_plugin.repos

RUN colcon build --cmake-args

# Use bash shell for ROS environment
SHELL ["/bin/bash", "-c"]

# Build workspace and clean up source
RUN source install/setup.bash && \
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/DDS-Visualizer-Plugin/install/lib && \
    export PATH=$PATH:~/DDS-Visualizer-Plugin/install/bin

# Set entrypoint and default command
CMD ["plotjuggler"]