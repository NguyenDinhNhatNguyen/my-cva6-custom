#ifndef TIMER_TARGET_H
#define TIMER_TARGET_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <unordered_map>

SC_MODULE(Timer_Target) {
    tlm_utils::simple_target_socket<Timer_Target> socket;
    sc_core::sc_out<bool> irq;

    uint32_t ctrl_reg = 0;           // Offset 0x4 (Bit 0: Enable)
    uint32_t compare_lower0_0 = 0;   // Offset 0x118 (Nguong dem nguoc)
    std::unordered_map<uint64_t, uint32_t> passive_regs;
    
    bool is_enabled = false;
    uint32_t saved_ticks = 0;
    sc_time last_start_time;
    sc_event irq_event;

    SC_CTOR(Timer_Target) : socket("socket") {
        socket.register_b_transport(this, &Timer_Target::b_transport);
        SC_METHOD(trigger_irq);
        sensitive << irq_event;
        dont_initialize();
    }

    uint32_t get_current_ticks(sc_time local_delay) {
        if (!is_enabled) return saved_ticks;
        sc_time current_sim_time = sc_time_stamp() + local_delay;
        return saved_ticks + ((current_sim_time - last_start_time) / sc_time(10, SC_NS));
    }

    void trigger_irq() {
        passive_regs[0x0] |= 0x1;
        irq.write(true);
        std::cout << "  [HW IRQ]: RV_TIMER bat trigger ngat | Time: " << sc_time_stamp() << "\n";
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
        if (trans.get_data_length() != 4) { trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE); return; }
        uint64_t offset = trans.get_address();
        uint32_t* data = reinterpret_cast<uint32_t*>(trans.get_data_ptr());

        if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {
            switch (offset) {
                case 0x4: 
                    ctrl_reg = data[0];
                    if ((ctrl_reg & 0x1) && !is_enabled) {
                        is_enabled = true;
                        last_start_time = sc_time_stamp() + delay;
                        if (compare_lower0_0 > saved_ticks) {
                            irq_event.notify((compare_lower0_0 - saved_ticks) * sc_time(10, SC_NS));
                        }
                    } else if (!(ctrl_reg & 0x1) && is_enabled) {
                        is_enabled = false;
                        saved_ticks = get_current_ticks(delay);
                        irq_event.cancel();
                    }
                    break;
                case 0x118: compare_lower0_0 = data[0]; break;
                case 0x0: 
                    passive_regs[0x0] &= ~data[0]; 
                    if (passive_regs[0x0] == 0) irq.write(false);
                    break;
                case 0x110: trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE); return;
                case 0x100: case 0x104: case 0x108: case 0x10c: case 0x114: case 0x11c:
                    passive_regs[offset] = data[0]; break;
                default: trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE); return;
            }
            delay += sc_time(10, SC_NS);
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
        } else if (trans.get_command() == tlm::TLM_READ_COMMAND) {
            switch (offset) {
                case 0x110: data[0] = get_current_ticks(delay); break; 
                case 0x4:   data[0] = ctrl_reg; break;
                case 0x118: data[0] = compare_lower0_0; break;
                case 0x0: case 0x100: case 0x104: case 0x108: case 0x10c: case 0x114: case 0x11c:
                    data[0] = passive_regs[offset]; break;
                default: data[0] = 0; trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE); return;
            }
            delay += sc_time(5, SC_NS);
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
        }
    }
};

#endif