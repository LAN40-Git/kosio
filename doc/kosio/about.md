# About

## Install
```shell
# On Ubuntu and Raspberry Pi OS you can get the libraries by running
sudo apt update
sudo apt install -y git cmake pkg-config liburing-dev build-essential
# Clone the repository and install it (Default CMAKE_INSTALL_PREFIX is /usr/local)
git clone git@github.com:LAN40-Git/kosio.git
cd kosio
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```
