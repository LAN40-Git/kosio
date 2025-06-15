## 环境依赖

1. **liburing**

   ```shell
   git clone https://github.com/axboe/liburing.git
   cd liburing
   ./configure
   make
   sudo make install
   ```

2. **json**

   ```shell
   sudo apt install nlohmann-json3-dev
   ```
   或者
   ```shell
   wget https://github.com/nlohmann/json/releases/latest/download/json.hpp
   sudo mv json.hpp /usr/local/include/nlohmann/json.hpp
   ```
   
3. **tbb**

   ```shell
   sudo apt-get install libtbb-dev
   ```
