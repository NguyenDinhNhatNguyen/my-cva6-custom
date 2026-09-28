#ifndef SIMPLE_ROUTER_H
#define SIMPLE_ROUTER_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <iostream>

SC_MODULE(Simple_Router) {
    tlm_utils::simple_target_socket<Simple_Router> target_socket;

    tlm_utils::simple_initiator_socket<Simple_Router> ram_socket;
    tlm_utils::simple_initiator_socket<Simple_Router> uart_socket;
    tlm_utils::simple_initiator_socket<Simple_Router> timer_socket;
    

    SC_CTOR(Simple_Router) {
        target_socket.register_b_transport(this, &Simple_Router::b_transport);
        target_socket.register_transport_dbg(this, &Simple_Router::transport_dbg);
    }

    unsigned int transport_dbg(tlm::tlm_generic_payload& trans) {
        sc_dt::uint64 addr = trans.get_address();
        if (addr >= 0x80000000 && addr < 0x81000000) { 
            trans.set_address(addr - 0x80000000);
            unsigned int ret = ram_socket->transport_dbg(trans);
            trans.set_address(addr);
            
            return ret;
        }
        return 0;
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
        sc_dt::uint64 addr = trans.get_address();

        
        if (addr >= 0x80000000 && addr < 0x80200000) { // RAM
            trans.set_address(addr - 0x80000000); 
            ram_socket->b_transport(trans, delay);
        } else if (addr >= 0x10000000 && addr < 0x10001000) { // UART
            trans.set_address(addr - 0x10000000);   
            uart_socket->b_transport(trans, delay); 
            trans.set_address(addr);             
        } else if (addr >= 0x10030000 && addr < 0x10031000) { // Timer
            trans.set_address(addr - 0x10030000);
            timer_socket->b_transport(trans, delay);
            trans.set_address(addr);
        } else {
            std::cout << "[Router] Địa chỉ không hợp lệ: 0x" << std::hex << addr << std::endl;
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }
    }
};

#endif