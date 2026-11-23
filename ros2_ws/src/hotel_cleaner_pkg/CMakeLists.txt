cmake_minimum_required(VERSION 3.8)
project(hotel_cleaner_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(smacc2 REQUIRED)
find_package(behaviortree_cpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread)

add_executable(smacc2_cleaner src/smacc2_cleaner.cpp)
ament_target_dependencies(smacc2_cleaner rclcpp smacc2)
target_link_libraries(smacc2_cleaner Boost::thread)

add_executable(bt_cleaner src/bt_cleaner.cpp)
ament_target_dependencies(bt_cleaner rclcpp behaviortree_cpp)

install(TARGETS
  smacc2_cleaner
  bt_cleaner
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
