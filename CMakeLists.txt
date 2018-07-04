cmake_minimum_required (VERSION 2.6)

project(msa-daemon LANGUAGES CXX)

add_executable(msa-daemon src/main.cpp src/MsaService.cpp src/MsaService.h src/MsaUiClient.cpp src/MsaUiClient.h src/MsaUiLauncher.h src/MsaUiHelper.cpp src/MsaUiHelper.h src/MsaErrors.h)
target_link_libraries(msa-daemon simpleipc msa file-util daemon-client-utils daemon-server-utils argparser)

install(TARGETS msa-daemon RUNTIME COMPONENT msa-daemon DESTINATION bin)
include(CPackSettings.cmake)