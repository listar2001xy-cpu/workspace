#pragma once
#include <stddef.h>
#include <stdint.h>

// core 层：单生产者/单消费者环形缓冲（纯逻辑，无硬件依赖，可在 PC 上 native 单测）。
// 典型用法：ADS1220 DRDY 中断（生产者）写入原始采样，主循环（消费者）读出后做抽取/换挡。
// 容量取 2 的幂（1 << CapacityLog2），用位掩码代替取模，避免整数除法。
//
// 线程模型：仅适用于单生产者 + 单消费者 + 单核（或 ISR 写 / 主循环读）场景。
// head_/tail_ 用 volatile 标记，用于 ISR 与主循环共享；并非多核内存屏障。
template <typename T, size_t CapacityLog2>
class RingBuffer {
    static_assert(CapacityLog2 > 0, "容量须 >= 2");

public:
    static constexpr size_t kCapacity = size_t(1) << CapacityLog2;  // 原始槽数

    // 生产者写入；缓冲满时返回 false 并递增溢出计数（数据完整性：绝不静默丢）。
    bool push(const T& item) {
        if (full()) {
            ++overflow_;
            return false;
        }
        buf_[head_] = item;
        head_ = (head_ + 1) & kMask;
        return true;
    }

    // 消费者读出；缓冲空时返回 false，不修改 out。
    bool pop(T& out) {
        if (empty()) {
            return false;
        }
        out = buf_[tail_];
        tail_ = (tail_ + 1) & kMask;
        return true;
    }

    size_t count() const { return (head_ - tail_) & kMask; }
    size_t capacity() const { return kCapacity - 1; }  // 保留一个空位以区分满/空
    bool full() const { return ((head_ + 1) & kMask) == tail_; }
    bool empty() const { return head_ == tail_; }
    uint32_t overflowCount() const { return overflow_; }

    void reset() {
        head_ = 0;
        tail_ = 0;
        overflow_ = 0;
    }

private:
    static constexpr size_t kMask = kCapacity - 1;

    T buf_[kCapacity];
    volatile size_t head_ = 0;  // 生产者写
    volatile size_t tail_ = 0;  // 消费者读
    uint32_t overflow_ = 0;
};
