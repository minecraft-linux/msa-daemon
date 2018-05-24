cmake_minimum_required (VERSION 2.6)

project(msa LANGUAGES CXX)

add_executable(msa-daemon src/main.cpp src/MsaService.cpp src/MsaService.h src/MsaUiClient.cpp src/MsaUiClient.h src/MsaUiLauncher.cpp src/MsaUiLauncher.h src/MsaUiHelper.cpp src/MsaUiHelper.h)
target_link_libraries(msa-daemon simpleipc msa file_util daemon_client_utils)