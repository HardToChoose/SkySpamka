TEMPLATE = app

CONFIG += windows c++11 warn_on

CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    Engine.cpp \
    GuiHelpers.cpp \
    MainWindow.cpp

win32
{
    LIBS += -lcomctl32 -lgdi32
    RC_FILE = skypamer.rc
}

QMAKE_CXXFLAGS_WARN_ON = -Wall -pedantic

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    Engine.h \
    Conversation.h \
    GuiHelpers.h \
    MainWindow.h \
    SendingInfo.h

