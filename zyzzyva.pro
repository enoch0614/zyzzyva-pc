TEMPLATE = app
LANGUAGE = C++

CONFIG += qt warn_on release

HEADERS += \
    src/AnalyzeQuizDialog.h \
    src/JudgeForm.h \
    src/MainWindow.h \
    src/NewQuizDialog.h \
    src/QuizEngine.h \
    src/QuizForm.h \
    src/SearchForm.h \
    src/SearchSpecForm.h \
    src/SettingsDialog.h \
    src/WordEngine.h \
    src/WordGraph.h \
    src/WordValidator.h \
    src/ZListViewItem.h

SOURCES += \
    src/AnalyzeQuizDialog.cpp \
    src/JudgeForm.cpp \
    src/main.cpp \
    src/MainWindow.cpp \
    src/NewQuizDialog.cpp \
    src/QuizEngine.cpp \
    src/QuizForm.cpp \
    src/SearchForm.cpp \
    src/SearchSpecForm.cpp \
    src/SettingsDialog.cpp \
    src/WordEngine.cpp \
    src/WordGraph.cpp \
    src/WordValidator.cpp \
    src/ZListViewItem.cpp

unix {
    VERSTR=$$system(cat VERSION)
    UI_DIR = .build/ui
    MOC_DIR = .build/moc
    OBJECTS_DIR = .build/obj
}

win32 {
    VERSTR=$$system(type VERSION)
    UI_DIR      = build/ui
    MOC_DIR     = build/moc
    OBJECTS_DIR = build/obj
}

MAKEFILE = zyzzyva.mak
DEFINES += VERSION=\"$$VERSTR\"
