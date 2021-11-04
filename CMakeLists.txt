# cmake_minimum_required(VERSION 3.0)
# project(test)

# find_package(PkgConfig REQUIRED)
# pkg_check_modules(Mosquitto REQUIRED IMPORTED_TARGET libmosquitto)

# include_directories(${mosquitto_SOURCE_DIR})
# target_link_libraries(${PROJECT_NAME} PkgConfig::Mosquitto)
# set(SOURCE_FILES mosquitto_pub_test.c mosquitto_sub_test.c mqtt_pub_demo.c mqtt_sub_demo.c)
# add_executable(${SOURCE_FILES} mosquitto.h)

# include(GNUInstallDirs)
# install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
# nstall(FILES mosquitto.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

cmake_minimum_required(VERSION 3.0)
project(mosquitto_pub_test)
project(mosquitto_sub_test)
project(mqtt_pub_demo)
project(mqtt_sub_demo)

find_package(Threads)
find_package(PkgConfig REQUIRED)
pkg_check_modules(mosquitto REQUIRED IMPORTED_TARGET libmosquitto)

set(mosquitto_pub_test mosquitto_pub_test.c)
set(mosquitto_sub_test mosquitto_sub_test.c)
set(mqtt_pub_demo mqtt_pub_demo.c)
set(mqtt_sub_demo mqtt_sub_demo.c)

add_executable(mosquitto_pub_test mosquitto_pub_test.c)
add_executable(mosquitto_sub_test mosquitto_sub_test.c)
add_executable(mqtt_pub_demo mqtt_pub_demo.c)
add_executable(mqtt_sub_demo mqtt_sub_demo.c)

target_link_libraries(mosquitto_pub_test PkgConfig::mosquitto)
target_link_libraries(mosquitto_sub_test PkgConfig::mosquitto pthread)
target_link_libraries(mqtt_pub_demo PkgConfig::mosquitto)
target_link_libraries(mqtt_sub_demo PkgConfig::mosquitto)

