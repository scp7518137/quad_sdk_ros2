cmake_minimum_required(VERSION 3.5)

project(quad_logger)

find_package(ament_cmake REQUIRED)

# Install the python scripts as executables (lib/<pkg> on install)
install(PROGRAMS
  scripts/bag_reader.py
  scripts/read_bag.py
  scripts/mouse_interface.py
  DESTINATION lib/${PROJECT_NAME})

ament_package()
