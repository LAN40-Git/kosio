```mermaid
graph TD
add_entry["添加定时事件"]
handle_expire_entries["时间轮找到非空槽位"]
execute["触发事件"]
add_entry 
--> 找到并加入对应时间轮
--> handle_expire_entries
--> execut


```

