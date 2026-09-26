#ifndef CVA6_INITIATOR_H
#define CVA6_INITIATOR_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/tlm_quantumkeeper.h>

SC_MODULE(CVA6_Initiator) {
    tlm_utils::simple_initiator_socket<CVA6_Initiator> socket;
    tlm_utils::tlm_quantumkeeper m_qk;

    SC_CTOR(CVA6_Initiator) : socket("socket") {
        m_qk.set_global_quantum(sc_time(1, SC_US));
        m_qk.reset();
        SC_THREAD(run_simulation);
    }

    void send_tx(tlm::tlm_command cmd, uint64_t addr, uint32_t& data) {
        tlm::tlm_generic_payload trans;
        sc_time local_delay = m_qk.get_local_time(); 
        
        trans.set_command(cmd);
        trans.set_address(addr);
        trans.set_data_ptr(reinterpret_cast<unsigned char*>(&data));
        trans.set_data_length(4); 
        trans.set_streaming_width(4);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        
        socket->b_transport(trans, local_delay);
        m_qk.set(local_delay);
        if (m_qk.need_sync()) { m_qk.sync(); }
    }

    void run_simulation() {
        std::cout << "[HETHONG]: Test CVA6_Initiator (CPU ao)...\n";
        uint32_t data = 0x1;
        send_tx(tlm::TLM_WRITE_COMMAND, 0x10001004, data); // Enable Timer
        m_qk.inc(sc_time(100, SC_NS));
        if (m_qk.need_sync()) m_qk.sync();
        send_tx(tlm::TLM_READ_COMMAND, 0x10001110, data);  // Doc Timer
        std::cout << "[HETHONG]: Hoan tat test.\n";
    }
};
#endif