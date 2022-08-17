
# ===== add translator ============================================ #
set(TS_FILES asaloader_zh_TW.ts)
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
# configure_file(${QM_FILES} ${CMAKE_SOURCE_DIR} COPYONLY)  # must build once than use this command
# ================================================================= #
# ===== add executable ============================================ #
qt_add_executable(${PROJECT_NAME} 
  main.cpp
  ${QM_FILES}
)
# set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
# set_target_properties(${PROJECT_NAME} PROPERTIES
#     WIN32_EXECUTABLE ON
#     # MACOSX_BUNDLE ON
# ) # this setting is for GUI applications

# ================================================================= #
# ===== add QT/custom libraries ============================================ #
add_subdirectory(loader)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::SerialPort)
target_link_libraries(${PROJECT_NAME} PRIVATE asaloaderlib)

# ================================================================= #
# ===== add install =====　The following script must only be executed at install time ======= #
set(deploy_script "${CMAKE_CURRENT_BINARY_DIR}/deploy_${PROJECT_NAME}.cmake")
set(executable_path "\${QT_DEPLOY_BIN_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>") 
file(GENERATE OUTPUT ${deploy_script} CONTENT "
# Including the file pointed to by QT_DEPLOY_SUPPORT ensures the generated
# deployment script has access to qt_deploy_runtime_dependencies()
include(\"${QT_DEPLOY_SUPPORT}\")
qt_deploy_runtime_dependencies(
    EXECUTABLE \"${executable_path}\"
    ADDITIONAL_EXECUTABLES \"${helper_app_path}\"
    GENERATE_QT_CONF
    VERBOSE
)")

# Omitting RUNTIME DESTINATION will install a non-bundle target to CMAKE_INSTALL_BINDIR,
# which coincides with the default value of QT_DEPLOY_BIN_DIR used above, './bin'.
# Installing macOS bundles always requires an explicit BUNDLE DESTINATION option.
install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}
        BUNDLE  DESTINATION .      # Install to CMAKE_INSTALL_PREFIX/MyApp.app/Contents/MacOS/MyApp
)
install(SCRIPT ${deploy_script})    # Add its runtime dependencies
# ================================================================= #