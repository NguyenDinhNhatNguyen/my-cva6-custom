#ifndef UART_TARGET_H
#define UART_TARGET_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <unordered_map>

SC_MODULE(UART_Target) {
    tlm_utils::simple_target_socket<UART_Target> socket;
    sc_core::sc_out<bool> irq;

    uint32_t status_reg = 0; // 0X14
    int tx_fifo_count = 0;   // Logic dem phan tu trong FIFO
    
    std::unordered_map<uint64_t, uint32_t> passive_regs;

    SC_CTOR(UART_Target) : socket("socket") {
        socket.register_b_transport(this, &UART_Target::b_transport);
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
        if (trans.get_data_length() != 4) { trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE); return; }
        uint64_t offset = trans.get_address();
        uint32_t* data = reinterpret_cast<uint32_t*>(trans.get_data_ptr());

        if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {
            switch (offset) {
                case 0x1c: 
                    if (tx_fifo_count < 16) {
                        tx_fifo_count++;
                        if (tx_fifo_count == 16) {
                            status_reg |= 0x20; 
                            passive_regs[0x0] |= 0x1; 
                            irq.write(true);          
                            std::cout << "  [HW IRQ]: UART Day FIFO | Time: " << sc_time_stamp() + delay << "\n";
                        }
                    }
                    break;
                case 0x0: 
                    passive_regs[0x0] &= ~data[0];
                    if (passive_regs[0x0] == 0) irq.write(false);
                    break;
                case 0x14: case 0x18: case 0x24: trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE); return;
                case 0x4: case 0x8: case 0xc: case 0x10: case 0x20: case 0x28: case 0x2c: case 0x30:
                    passive_regs[offset] = data[0]; break;
                default: trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE); return;
            }
            delay += sc_time(10, SC_NS);
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
        } else if (trans.get_command() == tlm::TLM_READ_COMMAND) {
            switch (offset) {
                case 0x1c: trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE); return;
                case 0x14: data[0] = status_reg; break;
                case 0x18: case 0x24: case 0x0: case 0x4: case 0x8: case 0xc: case 0x10: case 0x20: case 0x28: case 0x2c: case 0x30:
                    data[0] = passive_regs[offset]; break;
                default: data[0] = 0; trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE); return;
            }
            delay += sc_time(5, SC_NS);
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
        }
    }
};

#endif