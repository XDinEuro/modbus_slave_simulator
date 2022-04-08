find_package(Qt5 REQUIRED Core Widgets Network)

add_executable(modbus_slave_simulator
    main.cpp
	mainwindow.cpp
	QTcpServerInterface.cpp
)

target_include_directories(modbus_slave_simulator
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
	${PROJECT_BINARY_DIR}/include
)

target_link_libraries(modbus_slave_simulator
	PRIVATE
	Qt5::Widgets
	Qt5::Network
)

install(TARGETS modbus_slave_simulator DESTINATION "bin")
