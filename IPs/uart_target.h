#ifndef UART_TARGET_H
#define UART_TARGET_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <iostream>

SC_MODULE(UART_Target) {
    tlm_utils::simple_target_socket<UART_Target> socket;

    SC_CTOR(UART_Target) : socket("socket") {
        socket.register_b_transport(this, &UART_Target::b_transport);
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
        tlm::tlm_command cmd = trans.get_command();
        sc_dt::uint64 addr = trans.get_address();
        unsigned char* ptr = trans.get_data_ptr();

        if (cmd == tlm::TLM_WRITE_COMMAND) {
            if (addr == 0x1C) {
                std::cout << (char)(*ptr) << std::flush; 
            } 
        }
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    } 
};

#endif