sudo apt update
sudo apt install -y build-essential
sudo apt install -y cmake
sudo apt install -y libsdl2-dev

cd lib/enet-1.3.17
sudo chmod +x ./configure
./configure
make
sudo make install
sudo ldconfig
cd ../..

mkdir build
cd build
cmake ../
make
cd ..