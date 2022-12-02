sudo apt update
sudo apt install -y build-essential
sudo apt install -y cmake
sudo apt install -y libsdl2-dev

cd lib/enet-1.3.17
./configure
make
sudo make install
sudo rm /usr/local/lib/libenet.so.7
sudo ldconfig
cd ../..

mkdir build
cd build
cmake ../
make
cd ..