cmake_minimum_required (VERSION 2.6)

project(msa LANGUAGES CXX)

add_executable(msa-daemon src/main.cpp src/MsaService.cpp src/MsaService.h src/FileUtil.cpp src/FileUtil.h src/MsaUiClient.cpp src/MsaUiClient.h)
target_link_libraries(msa-daemon simpleipc msa)