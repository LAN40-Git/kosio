## Dependency

```shell
sudo apt update
sudo apt install -y build-essential cmake liburing-dev libtbb-dev nlohmann-json-dev
```


## INSTALL

```shell
git clone 
cd coruring
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
sudo ldconfig
```

## USE
Add the code to your CMakeLists.txt
```cmake
find_package(coruring REQUIRED)
target_link_libraries(your_app PRIVATE coruring)
```