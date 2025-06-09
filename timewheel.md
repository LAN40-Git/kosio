Entry

- expiration_time_：到期时间
- data_：回调结构体

**示例：**

事件A[expiratio_time=78ms]加入

计算对应槽位：

1. 计算层级：

   ```c++
   while (level + 1 < MAX_LEVEL && ticks >= PRECISION[level + 1]) {
       ++level;
       ticks /= SLOT_SIZE;
   }
   ```

2. 计算对应槽位

   ```
   
   ```

   
