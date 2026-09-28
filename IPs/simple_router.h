#ifndef SIMPLE_ROUTER_H
#define SIMPLE_ROUTER_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>

SC_MODULE(Simple_Router) {
    tlm_utils::simple_target_socket<Simple_Router> target_socket;
    tlm_utils::simple_initiator_socket<Simple_Router> uart_socket;
    tlm_utils::simple_initiator_socket<Simple_Router> timer_socket;

    SC_CTOR(Simple_Router) {
        target_socket.register_b_transport(this, &Simple_Router::b_transport);
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
        sc_dt::uint64 addr = trans.get_address(); 
        
        if (addr >= 0x10000000 && addr < 0x10001000) { // UART
            trans.set_address(addr - 0x10000000);   
            uart_socket->b_transport(trans, delay);              
        } 
        else if (addr >= 0x10030000 && addr < 0x10031000) { // Timer
            trans.set_address(addr - 0x10030000);
            timer_socket->b_transport(trans, delay);
        }     
        else {
            std::cerr << "[Router] Địa chỉ không hợp lệ: 0x" << std::hex << addr << std::dec << std::endl;
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        }

    }
};

#endif