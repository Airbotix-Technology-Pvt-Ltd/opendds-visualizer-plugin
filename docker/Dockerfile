FROM ghcr.io/opendds/opendds:latest-release
WORKDIR /DDS-Visualizer-Plugin/src

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
    libqt5svg5-dev \
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

RUN wget https://raw.githubusercontent.com/Airbotix-Technology-Pvt-Ltd/opendds-visualizer-plugin/opendds/opendds_visualizer_plugin.repos && \
    vcs import --recursive src < opendds_visualizer_plugin.repos

RUN colcon build --cmake-args

# Use bash shell 
SHELL ["/bin/bash", "-c"]

# Build workspace
RUN source install/setup.bash

# Create rtps.ini file
RUN mkdir -p /usr/local/share/dds && \
    cat > /usr/local/share/dds/rtps.ini << 'EOF'
[common]
DCPSGlobalTransportConfig=$file
DCPSDefaultDiscovery=DEFAULT_RTPS
DCPSPendingTimeout=30

[transport/the_rtps_transport]
transport_type=rtps_udp
EOF

# Add environment variables to .bashrc
RUN echo "source ~/DDS-Visualizer-Plugin/install/setup.bash" >> ~/.bashrc && \
    echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:~/DDS-Visualizer-Plugin/install/lib" >> ~/.bashrc && \
    echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/opt/OpenDDS/lib:/opt/OpenDDS/ACE_wrappers/lib" >> ~/.bashrc && \
    echo "export PATH=\$PATH:~/DDS-Visualizer-Plugin/install/bin" >> ~/.bashrc && \
    echo "export DDS_CONFIG_FILE=/usr/local/share/dds/rtps.ini" >> ~/.bashrc

# Set environment variables for current session
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/root/DDS-Visualizer-Plugin/install/lib:/opt/OpenDDS/lib:/opt/OpenDDS/ACE_wrappers/lib"
ENV PATH="${PATH}:/root/DDS-Visualizer-Plugin/install/bin"
ENV DDS_CONFIG_FILE="/usr/local/share/dds/rtps.ini"

# Set entrypoint and default command
CMD ["plotjuggler"]