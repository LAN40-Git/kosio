# Bug

1. 服务端两次连续 `write_all` 之后，客户端两次连续 `read_exact` 可能达到 **40ms** 
```c++
// 服务端
stream.write_all(...)
stream.write_all(...)

// 客户端
stream.read_exact()
stream.read_exact()
```

已解决：此问题是由于客户端套接字打开了Nagle算法，服务端