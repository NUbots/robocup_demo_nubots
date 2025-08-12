# NUbots dockerfile for running ROS2

FROM ubuntu:jammy-20250714 AS base

COPY usr/local/bin/pkg_install /usr/local/bin/pkg_install

# setup timezone
RUN echo 'Etc/UTC' > /etc/timezone && \
    ln -s /usr/share/zoneinfo/Etc/UTC /etc/localtime && \
    pkg_install tzdata

# install packages and deps
RUN apt update && apt upgrade -y

# get ros2 repositories for apt
RUN pkg_install software-properties-common \
    wget
RUN add-apt-repository universe && apt update
RUN wget -O /tmp/ros2-apt-source.deb "https://github.com/ros-infrastructure/ros-apt-source/releases/download/1.1.0/ros2-apt-source_1.1.0.jammy_all.deb"
RUN dpkg -i /tmp/ros2-apt-source.deb
RUN apt update && apt upgrade -y

# install ros2 and ros2 deps
RUN pkg_install ros-humble-desktop \
    ros-dev-tools

RUN pkg_install build-essential \
    git \
    python3-colcon-common-extensions \
    python3-colcon-mixin \
    python3-rosdep \
    python3-vcstool

RUN pkg_install ros-humble-backward-ros \
    ros-humble-behaviortree-cpp

# install unzip and curl to get code deps
RUN pkg_install unzip \
    curl

RUN curl --retry 5 -L -o /tmp/rerun_cpp_sdk.zip \
    https://github.com/rerun-io/rerun/releases/download/0.20.1/rerun_cpp_sdk.zip
WORKDIR /tmp
RUN wget -q -c https://developer.download.nvidia.com/compute/cuda/12.2.2/local_installers/cuda_12.2.2_535.104.05_linux.run
RUN wget -q -c -O /tmp/TensorRT-8.6.1.6.Linux.x86_64-gnu.cuda-12.0.tar.gz \
    https://developer.nvidia.com/downloads/compute/machine-learning/tensorrt/secure/8.6.1/tars/TensorRT-8.6.1.6.Linux.x86_64-gnu.cuda-12.0.tar.gz
RUN unzip rerun_cpp_sdk.zip
RUN sh cuda_12.2.2_535.104.05_linux.run --silent --toolkit

# install rerun_sdk
WORKDIR /tmp/rerun_cpp_sdk
RUN mkdir build && cd build && cmake .. && make && sudo make install

WORKDIR /tmp
RUN tar -xzf TensorRT-8.6.1.6.Linux.x86_64-gnu.cuda-12.0.tar.gz && sudo mv TensorRT-8.6.1.6 /usr/local
RUN git clone  https://github.com/BoosterRobotics/booster_robotics_sdk.git && cd booster_robotics_sdk && sudo ./install.sh

RUN pkg_install libgoogle-glog-dev \
    libopenblas-dev

# yoinked from robot; required to compile
COPY usr/local/include/booster_internal /usr/local/include/booster_internal

# Setup the sudo so it can be run without a password
COPY etc/sudoers.d/user /etc/sudoers.d/user
RUN chmod 440 /etc/sudoers.d/user

# Create the user and setup sudo so no password is required
ARG user_uid=1000
ARG user_gid=$user_uid
ARG dialout_gid=20
ARG video_gid=44
ARG render_gid=110

# useradd -G is not sufficient to add the nubots user to the dialout
# group as docker manages to lose supplementary groups when changing
# to the nubots user.
# Also need to add "--group-add dialout" to the docker run command in run.py
# Add user to uucp as well so people can directly test with arch/ubuntu/pop devices
RUN groupadd --gid $render_gid render \
    && groupadd --gid $user_gid nubots \
    && useradd --uid $user_uid --gid nubots -G audio,dialout,uucp,video,render -m nubots

USER nubots

# Without this git gets very angry and confused about the state of the user
RUN git config --global --add safe.directory '*'

# init ros2
RUN sudo rosdep init && rosdep update --rosdistro humble

# COPY ./ros_entrypoint.sh /

RUN echo 'source /opt/ros/humble/setup.bash' >> ~/.bashrc 
RUN echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
RUN echo 'export PATH=/usr/local/cuda/bin:$PATH' >> ~/.bashrc
RUN echo 'export LD_LIBRARY_PATH=/usr/local/TensorRT-8.6.1.6/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
RUN echo 'export PATH=/usr/local/TensorRT-8.6.1.6//bin:$PATH' >> ~/.bashrc

WORKDIR /home/nubots/ros2_ws
# ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["bash"]