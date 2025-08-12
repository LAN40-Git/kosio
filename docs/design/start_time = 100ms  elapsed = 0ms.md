start_time = 100ms | elapsed = 0ms

1. 加入一个 expiration_time = 110ms 的定时器，进入到wheel时，调用insert,参数 when = 10ms，调用level_for获取到0,通过levels_[0].add_entry传入事件，参数when=10ms，level调用slot_for获取到10ms在当前层级对应的槽位（when >> (0 * NUM_LEVELS) = 10）,最终，事件被插入第0层第10个槽位
2. 通过timer调用next_expiration_time，获取到下一个距离下一个最近事件到期所剩余的事件为10ms（这里假设当前时间与事件插入时间相同），于是睡眠10ms,在此期间无事件插入，10ms后，定时器苏醒
3. 定时器苏醒后，调用timer的handle_expiration_time，在其中调用wheel_的handle_expiration_time，传入的参数为当前时间-start_time，即10ms。wheel更新elapsed为传入的参数，