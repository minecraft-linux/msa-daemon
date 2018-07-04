set(CPACK_PACKAGE_NAME "msa-daemon")
set(CPACK_PACKAGE_VENDOR "mcpelauncher")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Microsoft Account login daemon")
set(CPACK_PACKAGE_CONTACT "https://github.com/minecraft-linux/msa-manifest/issues")
set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_CURRENT_BINARY_DIR};msa-daemon;msa-daemon;/")
set(CPACK_OUTPUT_CONFIG_FILE CPackConfig.cmake)

include(CPack)