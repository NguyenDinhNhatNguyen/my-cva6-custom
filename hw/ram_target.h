#ifndef RAM_TARGET_H
#define RAM_TARGET_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <vector>

SC_MODULE(Ram_Target) {
    tlm_utils::simple_target_socket<Ram_Target> socket;
    std::vector<uint8_t> mem;

    SC_CTOR(Ram_Target) : socket("socket") { 
        mem.resize(2 * 1024 * 1024, 0); // 2MB RAM
        socket.register_b_transport(this, &Ram_Target::b_transport);
        socket.register_transport_dbg(this, &Ram_Target::transport_dbg);

    }

    unsigned int transport_dbg(tlm::tlm_generic_payload& trans) {
        sc_time delay = SC_ZERO_TIME;
        b_transport(trans, delay); 
        if (trans.get_response_status() == tlm::TLM_OK_RESPONSE) {
            return trans.get_data_length();
        }
        return 0; 
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
        tlm::tlm_command cmd = trans.get_command();
        sc_dt::uint64 addr = trans.get_address();
        unsigned char* ptr = trans.get_data_ptr();
        unsigned int len = trans.get_data_length();

        if (addr + len > mem.size()) {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }

        if (cmd == tlm::TLM_READ_COMMAND) {
            memcpy(ptr, &mem[addr], len);
        } else if(cmd == tlm::TLM_WRITE_COMMAND) {
            memcpy(&mem[addr], ptr, len);
        }
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }
};

#endif