
cd `dirname $0`

echo "=================================="
echo "packing ..."

mkdir -p tmp/include
mkdir -p tmp/src/transmit

cp -av ../src/freq_util	tmp/src
cp -av ../src/rscode	tmp/src
cp -av ../src/queue		tmp/src
cp -av ../src/transmit/WaveBuilder.cpp	tmp/src/transmit
cp -av ../src/transmit/RSCodec.cpp		tmp/src/transmit
cp -av ../src/transmit/RSCodec.h		tmp/src/transmit
cp -av ../src/transmit/Builder.cpp		tmp/src/transmit
cp -av ../src/transmit/config.h			tmp/src/transmit
cp -av ../include/sonic	tmp/include

cd tmp
tar zcf wavelink_for_ios.tar.gz include src
mv wavelink_for_ios.tar.gz ../
cd ..
rm -fr tmp

