cmake_minimum_required(VERSION 3.16)
add_library(Game INTERFACE)

# Add custom target to publish the .NET project
add_custom_target(
  PublishDotNet ALL
  COMMAND dotnet publish -c Release -r linux-x64 --self-contained --packages
          .nuget/
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Publishing .NET Native AOT project")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/bin/Release//net9.0/linux-x64/native/libdotnet.so
  ${CMAKE_SOURCE_DIR}/resources/lib/libdotnet.so COPYONLY)

target_link_directories(Game INTERFACE ${CMAKE_SOURCE_DIR}/resources/lib)
target_include_directories(Game INTERFACE include)
target_link_libraries(Game INTERFACE dotnet)
