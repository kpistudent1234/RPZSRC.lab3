cmake_minimum_required(VERSION 3.8)
project(joint_control_sim)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

add_executable(p_regulator_sim src/p_regulator_sim.cpp)
ament_target_dependencies(p_regulator_sim rclcpp)

install(TARGETS
  p_regulator_sim
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
