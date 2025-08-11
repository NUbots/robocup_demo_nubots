#!/bin/bash

# Set these variables to match your robot's configuration
ROBOT_USER="booster"
ROBOT_IP="192.168.10.102"
ROBOT_PATH="/home/booster/Workspace/robocup_demo"

# Get the absolute path to the project root (one level up from this script)
SRC_PATH="$(cd "$(dirname "$0")/.." && pwd)"

# Sync the code to the robot
rsync -avz --delete \
  --exclude='.git' \
  --exclude='*.pyc' \
  --exclude='CMakeCache.txt' \
  --exclude='CMakeFiles/' \
  --exclude='build/' \
  "${SRC_PATH}/" "${ROBOT_USER}@${ROBOT_IP}:${ROBOT_PATH}/"

# Run the build script on the robot
ssh "${ROBOT_USER}@${ROBOT_IP}" "cd ${ROBOT_PATH} && ./scripts/build.sh"