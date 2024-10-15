#include "Util.h"

// Output value to I/O port, 8 bits
void outb(short Port, char Value){
    asm volatile("outb %1, %0" : : "dN" (Port), "a" (Value));
}


// Get input for a specified I/O port, 8 bits
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

unsigned short inw(unsigned short port) {
    unsigned short ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void memcpy(int *source, int *dest, int nbytes) { // copy memory from A to B
    int i;
    for (i = 0; i < nbytes; i++) {
        *(dest + i) = *(source + i);
    }
}

void sti() {
    asm volatile("sti");
}

void cli() {
    asm volatile("cli");
}


// Start of memory allocator.
// The allocated space spans 0x01000000-0x01100000, giving is exactly 1048576 bytes, or ~1MB of space.
// The blocks are set up as 8-bit locations, each giving a byte of how full the given block is used.
// The program uses a first-bit policy at the moment.
// Each location in the memory table is one byte long, and each bit corrosponds to one block.
// Each block is 128 bytes long, and must be deallocated at once.

uint8_t block_size = 128;
uint16_t num_blocks = 512;
uint8_t mem_table[512] = {0};
// careful! Making it too large can cause an issue!
// As it stands, a memory leak can cause the program to crash.
// Another issue: freeing up memory cannot happen, due to each block being a sort of stack. Must be de-allocated at once!

uint64_t malloc(uint32_t size_t) {
    // First function is for finding an avallible block for something under 256 bytes.
    // Declare counting variables
    // Note: division by zero is only if the denominator is 0
    // Changing block size does work, but shortens data
    uint16_t count = 0;
    uint16_t start = 0;
    uint16_t required = ((size_t-1)/block_size)+1;

    for (int s_block=0;s_block<num_blocks;s_block++) {
        for (int m_block=0;m_block<8;m_block++) {
            uint8_t m_val = mem_table[s_block] & (1 << m_block);
            // will be 0 if not used

            // If we have not began counting and the block is free, start
            if (!m_val) {
                if (!count) {
                    start = (s_block * 8) + m_block; // the Block number we start from (can be zero)
                }
                count++;
                if (count >= required) { // count is what we need
                    // set all blocks to be enabled
                    uint16_t start_s;
                    uint16_t start_m;
                    for (int i=0;i<required;i++) {
                        start_s = (start + i) / 8;
                        start_m = (start + i) % 8;
                        mem_table[start_s] |= (1 << start_m);
                    }
                    return (0x01000000 + (start * block_size));
                }

                
            } else {
                count = 0;
            }
        }
    }
    return 0;
}

void free(uint64_t ptr, uint32_t size_t) {
    uint16_t start = ((ptr - 0x01000000) / block_size);

    uint16_t required = ((size_t-1)/block_size)+1;

    uint16_t start_s;
    uint16_t start_m;

    for (int i=0;i<required;i++) {
        start_s = (start + i) / 8;
        start_m = (start + i) % 8;
        mem_table[start_s] &= ~(1 << start_m);
    }
}


char* int64_str(signed long num) {
    char* m_ptr = (char*)malloc(32);
    int m_i = 0;
    int ix = 0;
    bool z = false;

    // malloc failed, out of memory
    if (m_ptr == nullptr) {
        return 0;
    }
    // if the number is zero, easy
    if (!num) {
        m_ptr[0] = '0';
        m_ptr[1] = '\0';
        return m_ptr;
    } else if (num < 0) {
        z = true;
        num = -num;
    }
    char tmp[32];
    while (num) {
        tmp[m_i] = (num % 10) + '0';
        num /= 10;
        m_i++;
        ix++;
    }
    if (z) {
        m_ptr[0] = '-';
        m_i++;
    }
    while (ix) {
        m_ptr[m_i-ix] = tmp[ix-1];
        ix--;
    }
    m_ptr[m_i] = '\0';
    
    return m_ptr;
}

char* hex64_str(signed long num) {
    char* m_ptr = (char*)malloc(32);
    int m_i = 0;
    int ix = 0;
    bool z = false;

    // malloc failed, out of memory
    if (m_ptr == nullptr) {
        return 0;
    }
    // if the number is zero, easy
    if (!num) {
        m_ptr[0] = '0';
        m_ptr[1] = '\0';
        return m_ptr;
    } else if (num < 0) {
        z = true;
        num = -num;
    }
    char tmp[32];
    char code;
    char additive;
    while (num) {

        code = num & 0xf;
        if (code > 9) {
            additive = 'a';
            code = code - 10;
        } else {
            additive = '0';
        }
        tmp[m_i] = (code + additive);
        num >>= 4;
        m_i++;
        ix++;
    }
    if (z) {
        m_ptr[0] = '-';
        m_i++;
    }
    while (ix) {
        m_ptr[m_i-ix] = tmp[ix-1];
        ix--;
    }
    m_ptr[m_i] = '\0';
    
    return m_ptr;
}

uint64_t strlen(char* s) { // find string length (null-termed)
    int i = 0;
    while (s[i] != '\0') {
        i++;
    }
    return i;
}

char* strcat(char* a, char* b) {
    // lengths do not include null terminator
    uint64_t lenA = strlen(a);
    uint64_t lenB = strlen(b);


    char* m_ptr = (char*)malloc(lenA + lenB + 1);
    for (int i=0;i<lenA;i++) {
        m_ptr[i] = a[i];
    }
    for (int i=0;i<lenB;i++) {
        m_ptr[lenA+i] = b[i];
    }
    m_ptr[lenA + lenB] = '\0';
    
    return m_ptr;
}
