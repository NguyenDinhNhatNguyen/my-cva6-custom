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

    SC_CTOR(Simple_Router) : target_socket("target_socket"), uart_socket("uart_socket"), timer_socket("timer_socket") {
        target_socket.register_b_transport(this, &Simple_Router::b_transport);
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
        uint64_t addr = trans.get_address(); 
        
        if (addr >= 0x10000000 && addr < 0x10001000) { // UART
            trans.set_address(addr - 0x10000000);   
            uart_socket->b_transport(trans, delay); 
            trans.set_address(addr);             
        } 
        else if (addr >= 0x10001000 && addr < 0x10002000) { // Timer
            trans.set_address(addr - 0x10001000);
            timer_socket->b_transport(trans, delay);
            trans.set_address(addr);
        } 
        else {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        }
    }
};

#endif