#include <stdint.h>

#define UART_REG  (*(volatile uint32_t *)(0x10000000))
#define TIMER_REG    (*(volatile uint32_t *)(0x10030000))

void uart_putchar(char c) { 
    UART_REG = c; 
}

void uart_print(const char *str) { 
    while (*str) 
        uart_putchar(*str++); 
}

char uart_getchar() { 
    char c = 0;
    while (c == 0) {
        c = (char)UART_REG;
    }
    return c;
}

__attribute__((interrupt("machine")))
void trap_handler() {
    uint32_t mcause;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));

if (mcause == 0x8000000B) {
        uart_print("\n\n[IRQ] CPU nhận được ngắt từ Timer Hardware!\n");
        
        TIMER_REG = 0; // 0 = Xóa Ngắt
        
        uart_print(">> Vui lòng nhập tên của bạn: "); 
    }
}

void _start() {
    uart_print("\n===========================================\n");
    uart_print("  FIRMWARE TEST: UART (IO) + TIMER (IRQ) \n");
    uart_print("===========================================\n\n");

    __asm__ volatile("csrw mtvec, %0" :: "r"(trap_handler));
    
    __asm__ volatile("csrs mie, %0" :: "r"(1 << 11)); // Bật bit MEIE (Bit 11)
    
    __asm__ volatile("csrs mstatus, %0" :: "r"(1 << 3)); // Bật bit MIE (Bit 3)

    TIMER_REG = 1;

    uart_print(">> Vui lòng nhập tên của bạn: ");
    
    char name[50];
    int i = 0;
    while (1) {
        char c = uart_getchar(); 
        
        if (c == '\n' || c == '\r') {
            name[i] = '\0';
            break;
        }
        name[i++] = c;
        uart_putchar(c);
    }

    uart_print("\n\nXin chào, ");
    uart_print(name);

    while(1) {
    }
}