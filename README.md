# Robocup Demo Official Documentation
## Introduction
The Booster T1 Robocup official demo allows the robot to make autonomous decisions to kick the ball and complete the full Robocup match. It includes three programs: vision, brain, and game_controller.

- vision
    - The Robocup vision recognition program, based on Yolo-v8, detects objects such as robots, soccer balls, and the field, and calculates their positions in the robot's coordinate system using geometric relationships.
- brain
    - The Robocup decision-making program reads visual data and GameController game control data, integrates all available information, makes judgments, and controls the robot to perform corresponding actions, completing the match process.
- game_controller
    - Reads the game control data packets broadcast by the referee machine on the local area network, converts them into ROS2 topic messages, and makes them available for the brain to use.

## Install extra dependency
sudo apt-get install ros-humble-backward-ros

## Build and Run

### Note    
This repo support both jetpack 6.0 and 6.2. If the repo is deployed on jetpack 6.2 machine, please modify src/vision/config/vision.yaml to selelct correct trt model

vision.yaml for jetpack 6.0 machine
``` yaml
detection_model:
  model_path: "./src/vision/model/best_orin.engine"
  confidence_threshold: 0.2
```
vision.yaml for jetpack 6.2 machine
```yaml
detection_model:
  model_path: ""./src/vision/model/best_orin_10.3.engine"
  confidence_threshold: 0.2
```

To decide jetpack version, please execute `dpkg -l | grep jetpack` on host.

# Build the programs
./scripts/build.sh

# Run in the simultion environment
./scripts/sim_start.sh

# Run on the actual robot
./scripts/start.sh
```

## Documents
[Chinese Version](https://booster.feishu.cn/wiki/P5kJw6nDGib5wskZ3Yfc289lnIg)

[English Version](https://booster.feishu.cn/wiki/XY6Kwrq1bizif4kq7X9c14twnle)
```


# Simulation Installation for NUbots Team
The easiest way to install the simulation environment is to use the official NUbots documentation guided software version. Due to Intel RealSense only support to linux kernel 6.8 so please make sure your kernel version is 6.8 or lower. If you are using a newer kernel, please downgrade it to 6.8.

## Basic Requirements
1. Ubuntu 22.04
2. Nvidia GPU with driver version 535
3. Cuda 12.2
4. TensorRT 8.6.1
5. cuDNN 8.9.4
6. Omniverse Launcher
7. Isaac Sim 4.2  
8. ROS2 Humble 

### Install the basic tools
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    git \
    wget \
    curl 
```

## Install the Nvidia Environment
1. Install Nvidia GPU driver version 535
2. Install Cuda 12.2 which supports the GPU driver version 535
3. Install TensorRT 8.6.1
4. Install cuDNN 8.9.4

```bash
# Example for Linux x86_64 Ubuntu-22.04

# Recommended path(not mandatory)
mkdir ~/Workspace
cd ~/Workspace
mkdir booster tools tmp

# Install CUDA-12.2.2
cd ~/Workspace/tmp
wget https://developer.download.nvidia.com/compute/cuda/12.2.2/local_installers/cuda_12.2.2_535.104.05_linux.run
sudo sh cuda_12.2.2_535.104.05_linux.run --silent --toolkit
# If it fails, check the detailed information in /var/log/cuda-installer.log
less /var/log/cuda-installer.log
# If successful, it will be installed under /usr/local/cuda. You can check it with:
ls /usr/local/cuda
# Add necessary PATH and LD_LIBRARY_PATH
echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
echo 'export PATH=/usr/local/cuda/bin:$PATH' >> ~/.bashrc

# Install TensorRT 8.6.1
cd ~/Workspace/tmp
wget https://developer.nvidia.com/downloads/compute/machine-learning/tensorrt/secure/8.6.1/tars/TensorRT-8.6.1.6.Linux.x86_64-gnu.cuda-12.0.tar.gz
tar -xzf TensorRT-8.6.1.6.Linux.x86_64-gnu.cuda-12.0.tar.gz
sudo mv TensorRT-8.6.1.6 /usr/local
# Add necessary PATH 和 LD_LIBRARY_PATH
echo 'export LD_LIBRARY_PATH=/usr/local/TensorRT-8.6.1.6/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
echo 'export PATH=/usr/local/TensorRT-8.6.1.6//bin:$PATH' >> ~/.bashrc


# Downloading cuDNN from the official website requires authentication, so it can't be downloaded directly using wget.
# Please use a browser to search and download the corresponding version.
# wget https://developer.nvidia.com/downloads/compute/cudnn/secure/8.9.4/local_installers/12.x/cudnn-linux-x86_64-8.9.4.25_cuda12-archive.tar.xz
# Assuming the file cudnn-linux-x86_64-8.9.4.25_cuda12-archive.tar.xz has been downloaded
cd ~/Workspace/tmp
tar -xvf cudnn-linux-x86_64-8.9.4.25_cuda12-archive.tar.xz
sudo mv cudnn-linux-x86_64-8.9.4.25_cuda12-archive /usr/local/cudnn-8.9.4.25
echo 'export LD_LIBRARY_PATH=/usr/local/cudnn-8.9.4.25/lib:$LD_LIBRARY_PATH' >> ~/.bashrc

source ~/.bashrc
```


## Install Omniverse and Isaac Sim
The version RoboCup Demo based Isaac Sim is 4.2 which is no longer supported by Nvidia.You cannot download it from Omniverse automatic installer. You can download it from the official website.

Omniverse is also in changer this moment, I cannot find the Omniverse launcher. But I provide one downloaded in our Lab computer's download directory. 
```bash
# Download the Omniverse Launcher from the official website
mv ~/Downloads/OmniverseLauncher-linux.AppImage ~/ 
# Make it executable
chmod +x ~/OmniverseLauncher-linux.AppImage
# Install fuse if it is not installed
# sudo apt-get install fuse
# Run the Omniverse Launcher
./OmniverseLauncher-linux.AppImage
# It will then create in ./local
# Open the Omniverse Launcher and install Nucleus and xx
# Move the isaac sim from ~/Downloads/Isaac_Sim-4.2.0-linux-x86_64.tar.gz to ~/Workspace/booster/tools
# Download the Isaac Sim 4.2.0 from the official website
wget https://download.isaacsim.omniverse.nvidia.com/isaac-sim-standalone%404.2.0-rc.18%2Brelease.16044.3b2ed111.gl.linux-x86_64.release.zip
unzip ~/Downloads/isaac-sim-standalone%404.2.0-rc.18%2Brelease.16044.3b2ed111.gl.linux-x86_64.release.zip -d ~/.local/share/ov/pkg/isaac_sim-4.2.0/
```

## Install ROS2 Humble

From official ROS2 Humble installation guide, you can install ROS2 Humble on your system. Please follow the official guide to install ROS2 Humble.

https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debs.html

After installing ROS2 Humble, add the ROS2 Humble source to your bashrc file

```bash
echo 'source /opt/ros/humble/setup.bash' >> ~/.bashrc
source ~/.bashrc
```

Install colcon
```bash
sudo apt install python3-colcon-common-extensions
```

## Install Ros2 and Robocup-related Modules

```bash
# Install BehaviorTree-CPP（robocup_brain's dependency）
sudo apt install ros-humble-behaviortree-cpp
sudo apt install ros-humble-backward-ros
sudo apt update
sudo apt install libgoogle-glog-dev
sudo apt install libopenblas-dev


# Install rereun_cpp_sdk（robocup_brain's dependency）
cd ~/Workspace/tmp
wget https://github.com/rerun-io/rerun/releases/0.22.1/download/rerun_cpp_sdk.zip
unzip rerun_cpp_sdk.zip
cd rerun_cpp_sdk
mkdir build
cd build
cmake ..
make
sudo make install

# Install booster_robotics_sdk (robocup_brain and robocup_vision's dependency）
cd ~/Workspace/booster
git clone  https://github.com/BoosterRobotics/booster_robotics_sdk.git
cd booster_robotics_sdk
sudo ./install.sh

# Clone and compile robocup related code
cd ~/Workspace/booster
git clone https://github.com/BoosterRobotics/robocup_demo.git
cd robocup_demo
./scripts/build.sh
```

## Build TensorRT Model

```bash
mkdir -p ~/miniconda3
wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh -O ~/miniconda3/miniconda.sh
bash ~/miniconda3/miniconda.sh -b -u -p ~/miniconda3
rm ~/miniconda3/miniconda.sh


source ~/.bashrc

# Verify if Conda was installed successfully
conda --version


cd ~/Workspace/booster/robocup_demo
# To compile the TensorRT dependencies for the vision node, compile it first.
./scripts/build.sh
# Build engine
./scripts/vision/build_engine.sh

# If failed, please check conda env and activate trt
# If TOS error, please allow toc acceptance

# The script will first create the build environment using Conda and then start building the engine.
# This process may take some time, so please be patient while it completes.
# Once the process is completed successfully, you will find model.engine and model.wts in ./scripts/vision/exported_model folder.
ls ./scripts/vision/exported_model
# Copy model.engine to vision folder.
cp ./scripts/vision/exported_model/model.engine ./src/vision/model/
# Edit ~/Workspace/booster/robocup_demo/src/vision/config/vision.yaml 
# Edit ~/Workspace/booster/robocup_demo/src/vision/config/vision_smi.yaml 
# Modify the value of detection_model.model_path  to "./src/vision/model/model.engine"
```

## Run the Simulation
Download the following files from the official website and move them to ~/Workspace/tools directory.

isaac_package_0.0.6.run

booster-runer-full-0.0.10.run

Move the downloaded files to ~/Workspace/tools directory.

```bash
cd ~/Workspace/tools
chmod a+x isaac_package_0.0.6.run
chmod a+x booster-runner-full-0.0.10.run
```
## Start the Simulation Environment
```bash
# Open a new terminal to start Isaac Sim(you can also use double-click to start it)
cd ~/Workspace/tools
./isaac_package_0.0.6.run
# If the Isaac installation path is different, pass it as an argument like this:
# ./isaac_package_0.0.5.run ~/.loca/share/ov/pkg/isacc-sim-xxxx/python.sh
# The script will launch Isaac Sim's GUI. Please keep the terminal open and not close it.
# If you want to stop it, just close the terminal.

# Open a new terminal to start Booster Motion Coltrol(you can also use double-click to start it)
cd ~/Workspace/tools
./booster-runner-full-0.0.10.run
# The script will keep running. Please keep the terminal open and not close it.
# If you want to stop it, just close the terminal.

# Open a new terminal to start robocup
cd ~/Workspace/booster/robocup_demo
# If you have change the code, rebuild it.
./scripts/build.sh
# Start the programs.
./scripts/sim_start.sh
# This will launch brain_node, vision_node, game_controller_node, joystick_node in the background.
# You can check their running status via brain.log, vision.log, game_controller.log, joystick.log.
# If you want to stop them, just run ./scripts/sim_stop.sh
```
# Groot visualisation
```bash
# Download from the official website https://www.behaviortree.dev/groot/
# make it executable 
chmod +x ~/Downloads/Groot2-v1.6.1-x86_64.AppImage
# Default port is 1667
```

## Basic Official Documents to start robot in the simulation environment