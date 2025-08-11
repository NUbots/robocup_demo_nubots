#!/bin/bash
set -e

# setup ros2 environment
source "/opt/ros/humble/setup.bash" --
echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
echo 'export PATH=/usr/local/cuda/bin:$PATH' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/TensorRT-8.6.1.6/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
echo 'export PATH=/usr/local/TensorRT-8.6.1.6//bin:$PATH' >> ~/.bashrc

exec "$@"
