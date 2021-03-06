XCHIP_JNI_PATH := $(call my-dir)

include ${XCHIP_JNI_PATH}/../../dependencies/Utix/UtixJNI/jni/Android.mk

include $(CLEAR_VARS)

LOCAL_MODULE := XChip

XCHIP_SRC_DIR :=${XCHIP_JNI_PATH}/../../src
XCHIP_INCLUDE_DIR :=${XCHIP_JNI_PATH}/../../include
UTIX_INCLUDE_DIR :=${XCHIP_JNI_PATH}/../../dependencies/Utix/Utix/include
SDL2_INCLUDE_DIR :=${XCHIP_JNI_PATH}/../../dependencies/SDL2/ANDROID/include

LOCAL_CPPFLAGS := -std=c++11 -Wall -Wextra -fPIC -static -fno-exceptions

ifdef DEBUG
LOCAL_CPPFLAGS += -D_DEBUG -O0 -g  -fno-omit-frame-pointer
else
LOCAL_CPPFLAGS += -DNDEBUG -O3 -fomit-frame-pointer 
endif


LOCAL_C_INCLUDES += ${XCHIP_INCLUDE_DIR} \
                    ${UTIX_INCLUDE_DIR}  \
                    ${SDL2_INCLUDE_DIR}  \
                    ${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.9/include
                    

LOCAL_CPP_INCLUDES += ${LOCAL_C_INCLUDES} 


LOCAL_SRC_FILES := $(wildcard ${XCHIP_SRC_DIR}/Core/*.cpp)               \
                   $(wildcard ${XCHIP_SRC_DIR}/Plugins/SDLPlugins/*.cpp)


LOCAL_STATIC_LIBRARIES += Utix log

include $(BUILD_STATIC_LIBRARY)
