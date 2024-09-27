cmake_minimum_required(VERSION 3.16)
add_library(Game INTERFACE)

if(NOT DEFINED EMSCRIPTEN)
  # Add custom target to publish the .NET project
  add_custom_target(
    PublishDotNet ALL
    COMMAND dotnet publish Game.csproj -c Release -r browser-wasm --packages .nuget/
    # COMMAND dotnet publish Game.csproj -r linux-x64 -c Release --packages .nuget/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS Engine
    COMMENT "Publishing .NET Native AOT project")
else()
  add_custom_target(
    PublishDotNet ALL
    COMMAND dotnet publish Game_wasm.csproj -c Release -r browser-wasm --packages .nuget/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS Engine
    COMMENT "Publishing .NET Native AOT project")
endif()

