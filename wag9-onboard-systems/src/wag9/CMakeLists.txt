qt5_add_resources(wag9_resource_sources ../../res/Wag9.qrc)

add_executable(
    wag9_fs WIN32
    FaultStationMain.cpp
    Wag9Configuration.cpp Wag9Configuration.h
    Wag9Fs.cpp Wag9Fs.h
    Wag9PneumoPanel.cpp Wag9PneumoPanel.h
    Wag9CubiclePanel.cpp Wag9CubiclePanel.h
    Wag9SpriteSwitch.cpp Wag9SpriteSwitch.h
    Wag9FsInfoPanel.cpp Wag9FsInfoPanel.h
    Wag9FsServer.cpp Wag9FsServer.h
    Wag9PanelFactory.cpp Wag9PanelFactory.h
    Wag9Panels.cpp Wag9Panels.h
    Wag9Vehicle.h
    ${wag9_resource_sources}
)

target_include_directories(wag9_fs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(wag9_fs common)