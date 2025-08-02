# NUbots dockerfile for running ROS2

FROM ubuntu:jammy-20250714 AS base

COPY usr/local/bin/pkg_install /usr/local/bin/pkg_install

# setup timezone
RUN echo 'Etc/UTC' > /etc/timezone && \
    ln -s /usr/share/zoneinfo/Etc/UTC /etc/localtime && \
    pkg_install tzdata

# install packages and deps
RUN apt update && apt upgrade -y

RUN pkg_install software-properties-common \
    wget
RUN add-apt-repository universe && apt update
RUN wget -O /tmp/ros2-apt-source.deb "https://github.com/ros-infrastructure/ros-apt-source/releases/download/1.1.0/ros2-apt-source_1.1.0.jammy_all.deb"
RUN dpkg -i /tmp/ros2-apt-source.deb
RUN apt update && apt upgrade -y

RUN pkg_install ros-humble-desktop \
    ros-dev-tools

RUN pkg_install build-essential \
    git \
    python3-colcon-common-extensions \
    python3-colcon-mixin \
    python3-rosdep \
    python3-vcstool

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

RUN sudo rosdep init && rosdep update --rosdistro humble

COPY ./ros_entrypoint.sh /

RUN mkdir -p /home/nubots/ros2_ws/src
WORKDIR /home/nubots/ros2_ws
ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["bash"]