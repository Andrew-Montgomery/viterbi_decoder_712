TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    src/convolutional_encoder_712.cpp \
    src/viterbi_decoder_712.cpp

HEADERS += \
    src/bit_vector.h \
    src/convolutional_encoder_712.h \
    src/viterbi_decoder_712.h
