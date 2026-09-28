#ifndef TIMER_TARGET_H
#define TIMER_TARGET_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <unordered_map>
#include <sysc/kernel/sc_module.h>

SC_MODULE(Timer_Target) {
    tlm_utils::simple_target_socket<Timer_Target> socket;

    uint32_t ctrl_reg = 0;           // Offset 0x4 (Bit 0: Enable)
    uint32_t timer_v_lower0 = 0;     // Offset 0x110 (Gia tri dem hien tai)
    uint32_t compare_lower0_0 = 0;   // Offset 0x118 (Nguong dem nguoc)

    std::unordered_map<uint64_t, uint32_t> passive_regs;

    SC_CTOR(Timer_Target) : socket("socket") {
        socket.register_b_transport(this, &Timer_Target::b_transport);
        
        SC_THREAD(timer_tick);
    }

    void timer_tick() {
        while (true) {
            wait(10, SC_NS); 
            if (ctrl_reg & 0x1) { timer_v_lower0++; }
        }
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
        tlm::tlm_command cmd = trans.get_command();
        if (trans.get_data_length() != 4) {
            trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
            return;
        }

        uint64_t offset = trans.get_address();
        uint32_t* data = reinterpret_cast<uint32_t*>(trans.get_data_ptr());

        if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {
            switch (offset) {
                case 0x4:   // CTRL
                    ctrl_reg = data[0]; 
                    break;
                case 0x118: // COMPARE_LOWER0_0
                    compare_lower0_0 = data[0]; 
                    break;
                case 0x110: // TIMER_V_LOWER0
                    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE); 
                    return;
                
                case 0x0:   // ALERT_TEST
                case 0x100: // INTR_ENABLE0
                case 0x104: // INTR_STATE0
                case 0x108: // INTR_TEST0
                case 0x10c: // CFG0
                case 0x114: // TIMER_V_UPPER0
                case 0x11c: // COMPARE_UPPER0_0
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
                case 0x4:   
                    data[0] = ctrl_reg; break;
                case 0x110: 
                    data[0] = timer_v_lower0; break; 
                case 0x118: 
                    data[0] = compare_lower0_0; break;
                
                case 0x0: 
                case 0x100: 
                case 0x104: 
                case 0x108: 
                case 0x10c: 
                case 0x114: 
                case 0x11c:
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