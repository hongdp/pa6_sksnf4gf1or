cd hadoop-example/mysrc/
make clean
make
./invIndexTest.sh
rm -rf ../../invoutput
mkdir ../../invoutput
cp -r output/* ../../invoutput
cp output/part-r-00000 ../../pa6_CPP/invertedIndex
cd ../..