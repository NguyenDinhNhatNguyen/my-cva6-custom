#ifndef CVA6_INITIATOR_H
#define CVA6_INITIATOR_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/tlm_quantumkeeper.h>

SC_MODULE(CVA6_Initiator) {
    tlm_utils::simple_initiator_socket<CVA6_Initiator> socket;
    sc_in<bool> irq_in;

    SC_CTOR(CVA6_Initiator) : socket("socket") {
        SC_THREAD(cpu_thread);
    }

    void write_bus(uint64_t addr, uint32_t data) {
        tlm::tlm_generic_payload trans;
        sc_time delay = sc_time(10, SC_NS); 
        trans.set_command(tlm::TLM_WRITE_COMMAND);
        trans.set_address(addr);
        trans.set_data_ptr(reinterpret_cast<unsigned char*>(&data));
        trans.set_data_length(4);
        trans.set_streaming_width(4);
        trans.set_byte_enable_ptr(nullptr);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

        socket->b_transport(trans, delay);
        wait(delay);
        }
    
    void cpu_thread() {
        wait(1, SC_US);

        write_bus(0x1000001C, 'D');
        write_bus(0x1000001C, 'O');
        write_bus(0x1000001C, 'N');
        write_bus(0x1000001C, 'E');
        write_bus(0x1000001C, '\n');

        std::cout << "[CPU Ao] Cấu hình Timer... " << std::endl;
        write_bus(0x10030118, 500000); 
        write_bus(0x10030004, 1);     

        std::cout << "[CPU Ao] Chuyển sang chế độ WFI (Chờ ngắt)..." << std::endl;
        while (irq_in.read() == false) {
            wait(irq_in.value_changed_event());
        }

        write_bus(0x1000001C, 'I');
        write_bus(0x1000001C, 'R');
        write_bus(0x1000001C, 'Q');
        write_bus(0x1000001C, '!');
        write_bus(0x1000001C, '\n');
        
        sc_stop();
    }
};
#endif