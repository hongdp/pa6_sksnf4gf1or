cd pagerank
make clean
make
./eecs485pa5p 0.85 -k 50 ../hadoop-example/dataset/mining.edges.xml PageRank.txt
cp PageRank.txt ../pa6_CPP/
rm -rf ../proutput
mkdir ../proutput
cp PageRank.txt ../proutput/
cd ..
