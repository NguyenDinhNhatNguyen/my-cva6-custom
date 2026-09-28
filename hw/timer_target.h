#ifndef TIMER_TARGET_H
#define TIMER_TARGET_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

SC_MODULE(Timer_Target) {
    tlm_utils::simple_target_socket<Timer_Target> socket;
    sc_out<bool> irq_out;

    uint32_t ctrl_reg;           
    uint32_t compare_reg;   
    
    SC_CTOR(Timer_Target) : socket("socket"), ctrl_reg(0), compare_reg(0) {
        socket.register_b_transport(this, &Timer_Target::b_transport);
        SC_THREAD(timer_thread);
        irq_out.initialize(false);
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
        tlm::tlm_command cmd = trans.get_command();
        sc_dt::uint64 addr = trans.get_address();
        unsigned char* ptr = trans.get_data_ptr();

        if (cmd == tlm::TLM_WRITE_COMMAND) {
            uint32_t val = *((uint32_t*)ptr);
            if (addr == 0x04) {
                ctrl_reg = val;
            } else if (addr == 0x118) {
                compare_reg = val;
            }

            // std::cout << "[Timer] Nhận cấu hình từ CPU - Offset: 0x" << std::hex << addr << " | Giá trị: " << std::dec << val << std::endl;
        }    
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    void timer_thread() {
        while (true) {
            wait(1, SC_US); 
            if (ctrl_reg == 1 && compare_reg > 0) {
                // std::cout << "[Timer] Bắt đầu đếm " << compare_reg << " ns..." << std::endl;
                wait(compare_reg, SC_NS);
                // std::cout << "[Timer] Ngắt Timer | Time: " << sc_time_stamp() << std::endl;

                irq_out.write(true);
                ctrl_reg = 0;
            }
            else if (ctrl_reg == 0) {
                irq_out.write(false);
            }    
        }
    }
};

#endif