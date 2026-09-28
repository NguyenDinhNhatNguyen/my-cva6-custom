#include "cva6_initiator.h"
#include "uart_target.h"
#include "timer_target.h"
#include "simple_router.h"

int sc_main(int argc, char* argv[]) {
    CVA6_Initiator  cpu("CPU");
    UART_Target     uart("UART");
    Timer_Target    timer("TIMER_IP");
    Simple_Router   router("BUS_ROUTER");
    
    // Chua co Router, test bang cach noi thang (CPU -> UART hoac CPU -> Timer)
    // cpu.socket.bind(uart.socket);
    // cpu.socket.bind(timer.socket);

    // Khi co Router, CPU -> Router (giua Initiator va Targets)
    cpu.socket.bind(router.target_socket);
    // Router phan luong ra UART (0x10000000) va Timer (0x10001000)
    router.uart_socket.bind(uart.socket);
    router.timer_socket.bind(timer.socket);

    sc_start();
    return 0;
}