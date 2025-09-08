# About

## Install

```shell
# On Ubuntu and Raspberry Pi OS you can get the libraries by running
sudo apt update
sudo apt install -y git cmake pkg-config liburing-dev build-essential
# Clone the repository and install it
git clone git@github.com:LAN40-Git/kosio.git
cd kosio
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
```

## Usage
```cmake
find_package(kosio REQUIRED)
target_link_libraries(your_app PRIVATE kosio::kosio)
```

## Uninstall
```shell
sudo rm -rf /usr/local/include/kosio
sudo rm -rf /usr/local/lib/libkosio.*
```
