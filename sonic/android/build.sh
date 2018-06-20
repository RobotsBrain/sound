

check_ndk_path()
{
    if [ "${NDK_ROOT}" = "" ]; then
        echo '''Please set the android-ndk path first, for example: export NDK_ROOT=/path/to/android-ndk'''
        exit 1
    fi
}

check_ndk_path

cd `dirname $0`
${NDK_ROOT}/ndk-build V=1

echo "=================================="
echo "packing ..."

cp -av libs/* ../demo/app/src/main/jniLibs/
cp -av src/* ../demo/app/src/main/java/

svn export ../demo

tar zcf wavelink_for_android.tar.gz libs src demo
rm -fr libs obj demo

