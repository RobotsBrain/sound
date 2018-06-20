#  Created by linyehui on 2014-04-10.
#  Copyright (c) 2014年 linyehui. All rights reserved.
#

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include \


LOCAL_SRC_FILES :=  \
                  ../../src/freq_util/bb_freq_util.cpp \
                  ../../src/rscode/rscode.cpp \
                  ../../src/queue/queue.cpp \
                  ../../src/transmit/WaveBuilder.cpp \
                  ../../src/transmit/RSCodec.cpp \
                  ../../src/transmit/Builder.cpp \
                  com_arcsoft_wavelink_WaveBuilder.cpp

LOCAL_MODULE    := wavelink
APP_OPTIM := debug

LOCAL_CFLAGS += -g -Wno-multichar
LOCAL_CFLAGS += -D SONIC_TARGET_OS_ANDROID
LOCAL_CFLAGS += -D ANDROID_SMP=0
          
# Include the static libraries pulled in via Android Maven plugin makefile (see include below)
LOCAL_STATIC_LIBRARIES := $(ANDROID_MAVEN_PLUGIN_LOCAL_STATIC_LIBRARIES)
          
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
            
include $(BUILD_SHARED_LIBRARY)

# Include the Android Maven plugin generated makefile
# Important: Must be the last import in order for Android Maven Plugins paths to work
include $(ANDROID_MAVEN_PLUGIN_MAKEFILE)
