#include "cva6_initiator.h"
#include "uart_target.h"
#include "timer_target.h"
#include "simple_router.h"

int sc_main(int argc, char* argv[]) {
    CVA6_Initiator  cpu("CVA6_CPU");
    UART_Target     uart("UART_IP");
    Timer_Target    timer("TIMER_IP");
    Simple_Router   router("BUS_ROUTER");

    sc_signal<bool> uart_irq("uart_irq");
    sc_signal<bool> timer_irq("timer_irq");

    cpu.socket.bind(router.target_socket);
    router.uart_socket.bind(uart.socket);
    router.timer_socket.bind(timer.socket);

    uart.irq.bind(uart_irq);
    timer.irq.bind(timer_irq);

    sc_start();
    return 0;
}