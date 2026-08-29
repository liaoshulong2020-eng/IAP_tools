cmake_minimum_required(VERSION 3.21)
project(HLD_CANFDToolPro VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(HLD_CANFDToolPro
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/modelitem.h
    src/mytablemodel.cpp
    src/mytablemodel.h
    src/candevice.cpp
    src/candevice.h
    src/configmanager.cpp
    src/csvwriter.cpp
    src/iapupgrade.cpp
    src/configmanager.h
    src/csvwriter.h
    src/iapupgrade.h
    src/recvthread.cpp
    src/recvthread.h
    src/sendthread.cpp
    src/sendthread.h
    src/inc/controlcanfd.h
    src/dialogs/opendlg.cpp       src/dialogs/opendlg.h
    src/dialogs/initdlg.cpp       src/dialogs/initdlg.h
    src/dialogs/paradialog.cpp    src/dialogs/paradialog.h
    src/dialogs/filterdialog.cpp  src/dialogs/filterdialog.h
    src/dialogs/siftdialog.cpp    src/dialogs/siftdialog.h
    src/dialogs/valuedialog.cpp   src/dialogs/valuedialog.h
    src/dialogs/infodialog.cpp    src/dialogs/infodialog.h
    src/dialogs/listsenddlg.cpp   src/dialogs/listsenddlg.h
    src/dialogs/filedlg.cpp       src/dialogs/filedlg.h
    src/dialogs/savefiledlg.cpp   src/dialogs/savefiledlg.h
    src/dialogs/datadlg.cpp       src/dialogs/datadlg.h
    src/dialogs/updatedlg.cpp     src/dialogs/updatedlg.h
    resources/resources.qrc
    resources/version.rc
)

target_include_directories(HLD_CANFDToolPro PRIVATE src src/inc src/dialogs)
target_link_libraries(HLD_CANFDToolPro PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

if(WIN32)
    target_compile_definitions(HLD_CANFDToolPro PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()
