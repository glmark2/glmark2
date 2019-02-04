#!/bin/bash

# Copyright 2019 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if [ -z "${ANDROID_SDK}" ];
then echo "Please set ANDROID_SDK, exiting"; exit 1;
else echo "ANDROID_SDK is ${ANDROID_SDK}";
fi

if [ -z "${ANDROID_NDK}" ];
then echo "Please set ANDROID_NDK, exiting"; exit 1;
else echo "ANDROID_NDK is ${ANDROID_NDK}";
fi

if [ -z "${JAVA_HOME}" ];
then echo "Please set JAVA_HOME, exiting"; exit 1;
else echo "JAVA_HOME is ${JAVA_HOME}";
fi

BUILD_TOOLS_VERSION=26.0.1
BUILD_TOOLS=$ANDROID_SDK/build-tools/$BUILD_TOOLS_VERSION
if [ ! -d "${BUILD_TOOLS}" ];
then echo "Please download correct build-tools version: ${BUILD_TOOLS_VERSION}, exiting"; exit 1;
else echo "BUILD_TOOLS is ${BUILD_TOOLS}";
fi

set -ev

GLMARK2_BUILD_DIR=$PWD
GLMARK2_BASE_DIR=$GLMARK2_BUILD_DIR/..
echo GLMARK2_BASE_DIR="${GLMARK2_BASE_DIR}"
echo GLMARK2_BUILD_DIR="${GLMARK2_BUILD_DIR}"

# Android 16 is the minSdkVersion supported
ANDROID_JAR=$ANDROID_SDK/platforms/android-16/android.jar

function create_APK() {
    mkdir -p bin/lib obj
    cp -r $GLMARK2_BUILD_DIR/libs/* $GLMARK2_BUILD_DIR/bin/lib
    cp -r $GLMARK2_BASE_DIR/data $GLMARK2_BUILD_DIR/bin/assets
    $BUILD_TOOLS/aapt package -f -m -S res -J src -M AndroidManifest.xml -I $ANDROID_JAR
    $JAVA_HOME/bin/javac -d ./obj -source 1.7 -target 1.7 -bootclasspath $JAVA_HOME/jre/lib/rt.jar -classpath $ANDROID_JAR:obj -sourcepath src src/org/linaro/glmark2/*.java
    $BUILD_TOOLS/dx --dex --output=bin/classes.dex ./obj
    $BUILD_TOOLS/aapt package -f -M AndroidManifest.xml -S res -I $ANDROID_JAR -F $1-unaligned.apk bin
    $JAVA_HOME/bin/jarsigner -verbose -keystore ~/.android/debug.keystore -storepass android -keypass android  $1-unaligned.apk androiddebugkey
    $BUILD_TOOLS/zipalign -f 4 $1-unaligned.apk $1.apk
}

#
# build native libraries
#
$ANDROID_NDK/build/ndk-build -j $cores

#
# build glmark2 APK
#
create_APK glmark2

echo Builds succeeded
exit 0
