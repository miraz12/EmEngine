cmake_minimum_required(VERSION 3.16)
add_library(Game INTERFACE)

if(NOT DEFINED EMSCRIPTEN)
  # Add custom target to publish the .NET project
  add_custom_target(
    PublishDotNet ALL
    COMMAND rm -r bin && dotnet publish -r linux-x64 -c Release --packages
            .nuget/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS Engine
    COMMENT "Publishing .NET Native AOT project")
else()
  add_custom_target(
    PublishDotNet ALL
    COMMAND dotnet publish -c Release -r browser-wasm --packages .nuget/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Publishing .NET Native AOT project")
endif()
