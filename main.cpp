#include <systemc.h>
#include <tlm.h>

#include "cva6_initiator.h"
#include "uart_target.h"
#include "timer_target.h"
#include "simple_router.h"

int sc_main(int argc, char* argv[]) {
    CVA6_Initiator  cpu("CPU");
    UART_Target     uart("UART");
    Timer_Target    timer("Timer");
    Simple_Router   router("Router");

    sc_signal<bool> irq_signal;

    cpu.socket.bind(router.target_socket);
    router.uart_socket.bind(uart.socket);
    router.timer_socket.bind(timer.socket);

    timer.irq_out.bind(irq_signal);
    cpu.irq_in.bind(irq_signal);

    std::cout << "Starting simulation..." << std::endl;
    sc_start();
    std::cout << "Simulation finished." << std::endl;

    return 0;
}