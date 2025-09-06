## Dependency

```shell
sudo apt update
sudo apt install -y build-essential cmake liburing-dev pkg-config
```

## INSTALL

```shell
git clone git@github.com:LAN40-Git/kosio.git
cd kosio
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
```

## USE
Add the code to your CMakeLists.txt

```cmake
find_package(. REQUIRED)
target_link_libraries(your_app PRIVATE kosio::kosio)
```
