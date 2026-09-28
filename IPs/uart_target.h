#ifndef UART_TARGET_H
#define UART_TARGET_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <unordered_map>

SC_MODULE(UART_Target) {
    tlm_utils::simple_target_socket<UART_Target> socket;

    uint32_t status_reg = 0; // 0X14
    int tx_fifo_count = 0;   // Logic dem phan tu trong FIFO

    std::unordered_map<uint64_t, uint32_t> passive_regs;

    SC_CTOR(UART_Target) : socket("socket") {
        socket.register_b_transport(this, &UART_Target::b_transport);
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
        if (trans.get_data_length() != 4) {
            trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE); 
            return;
        }

        uint64_t offset = trans.get_address();
        uint32_t* data = reinterpret_cast<uint32_t*>(trans.get_data_ptr());

        if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {
            switch (offset) {
                case 0x1c: // WDATA 
                    if (tx_fifo_count < 16) {
                        tx_fifo_count++;
                        if (tx_fifo_count == 16) {
                            status_reg |= 0x20; // Bat co TX_FIFO_FULL (bit 5)
                        }
                    } else {
                        // FIFO day, Drop du lieu
                    }
                    break;

                case 0x14: // STATUS
                case 0x18: // RDATA
                case 0x24: // FIFO_STATUS
                    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE); // Block write
                    return;

                case 0x0:  // INTR_STATE
                case 0x4:  // INTR_ENABLE
                case 0x8:  // INTR_TEST
                case 0xc:  // ALERT_TEST
                case 0x10: // CTRL
                case 0x20: // FIFO_CTRL
                case 0x28: // OVRD
                case 0x2c: // VAL
                case 0x30: // TIMEOUT_CTRL
                    passive_regs[offset] = data[0];
                    break;

                default:
                    trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
                    return;
            }
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
        } 
     else if (trans.get_command() == tlm::TLM_READ_COMMAND) {
            switch (offset) {
                case 0x1c: 
                    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE); // Block read
                    return;

                case 0x14:
                    data[0] = status_reg;
                    break;

                case 0x0: 
                case 0x4: 
                case 0x8: 
                case 0xc: 
                case 0x10: 
                case 0x18: 
                case 0x20: 
                case 0x24: 
                case 0x28: 
                case 0x2c: 
                case 0x30:
                    data[0] = passive_regs[offset];
                    break;

                default:
                    data[0] = 0;
                    trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
                    return;
            }
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
        }
    }
};

#endif