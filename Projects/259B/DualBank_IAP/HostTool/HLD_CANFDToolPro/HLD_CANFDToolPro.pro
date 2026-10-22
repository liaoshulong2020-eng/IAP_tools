QT       += core gui widgets

CONFIG   += c++17
TARGET    = HLD_CANFDToolPro
TEMPLATE  = app

# 关闭 Qt6 下已废弃但仍可用的部分 API 告警（不影响编译）
DEFINES  += QT_DISABLE_DEPRECATED_BEFORE=0x050000

INCLUDEPATH += src src/inc src/dialogs

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/mytablemodel.cpp \
    src/candevice.cpp \
    src/configmanager.cpp \
    src/csvwriter.cpp \
    src/iapupgrade.cpp \
    src/recvthread.cpp \
    src/sendthread.cpp \
    src/dialogs/opendlg.cpp \
    src/dialogs/initdlg.cpp \
    src/dialogs/paradialog.cpp \
    src/dialogs/filterdialog.cpp \
    src/dialogs/siftdialog.cpp \
    src/dialogs/valuedialog.cpp \
    src/dialogs/infodialog.cpp \
    src/dialogs/listsenddlg.cpp \
    src/dialogs/filedlg.cpp \
    src/dialogs/savefiledlg.cpp \
    src/dialogs/datadlg.cpp \
    src/dialogs/updatedlg.cpp

HEADERS += \
    src/mainwindow.h \
    src/modelitem.h \
    src/mytablemodel.h \
    src/candevice.h \
    src/configmanager.h \
    src/csvwriter.h \
    src/iapupgrade.h \
    src/recvthread.h \
    src/sendthread.h \
    src/inc/controlcanfd.h \
    src/dialogs/opendlg.h \
    src/dialogs/initdlg.h \
    src/dialogs/paradialog.h \
    src/dialogs/filterdialog.h \
    src/dialogs/siftdialog.h \
    src/dialogs/valuedialog.h \
    src/dialogs/infodialog.h \
    src/dialogs/listsenddlg.h \
    src/dialogs/filedlg.h \
    src/dialogs/savefiledlg.h \
    src/dialogs/datadlg.h \
    src/dialogs/updatedlg.h

RESOURCES += resources/resources.qrc
RC_FILE = resources/version.rc

# 发布构建时把运行库拷到可执行目录（可选）
# win32:CONFIG(release, debug|release) { ... }
