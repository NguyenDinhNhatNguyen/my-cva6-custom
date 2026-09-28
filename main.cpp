#include <systemc.h>
#include <tlm.h>

#include "cpu_models/riscv_vp/include/riscv_vp_wrapper.h"
#include "uart_target.h"
#include "timer_target.h"
#include "simple_router.h"
#include "ram_target.h"

SC_MODULE(Irq_Adapter) {
    sc_in<bool> irq_in;
    cdc::cpu::riscv_vp_cpu* cpu_ptr; 

    SC_CTOR(Irq_Adapter) {
        SC_METHOD(on_irq_change);
        sensitive << irq_in;
        dont_initialize(); 
    }

    void on_irq_change() {
        if (cpu_ptr) {
            cpu_ptr->set_irq(11, irq_in.read()); 
        }
    }
};

int sc_main(int argc, char* argv[]) {
    cdc::cpu::riscv_vp_cpu      cpu("CPU");
    UART_Target                 uart("UART");
    Timer_Target                timer("Timer");
    Simple_Router               router("Router");
    Ram_Target                  ram("RAM");

    Irq_Adapter irq_adapter("IrqAdapter"); 
    irq_adapter.cpu_ptr = &cpu; // Con trở CPU được Adapter quản lý

    sc_signal<bool> irq_signal;

    cpu.data_bus().bind(router.target_socket);
    router.ram_socket.bind(ram.socket);
    router.uart_socket.bind(uart.socket);
    router.timer_socket.bind(timer.socket);

    // Timer -> Dây tín hiệu -> Adapter -> Hàm C++ của CPU
    timer.irq_out.bind(irq_signal);
    irq_adapter.irq_in.bind(irq_signal);

    std::string elf_file = "firmware.elf";
    if (argc > 1) {
        elf_file = argv[1];
    }
    cpu.load_elf(elf_file.c_str());

    std::cout << "Starting simulation..." << std::endl;
    sc_start();
    std::cout << "Simulation finished." << std::endl;

    return 0;
};