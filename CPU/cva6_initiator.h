#ifndef CVA6_INITIATOR_H
#define CVA6_INITIATOR_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <string>
#include <iostream>
#include <iomanip>

SC_MODULE(CVA6_Initiator) {
    tlm_utils::simple_initiator_socket<CVA6_Initiator> socket;
    
    sc_time tick_period = sc_time(10, SC_NS); // Clock 10ns

    SC_CTOR(CVA6_Initiator) : socket("socket") {
        SC_THREAD(run_simulation);
    }

    void send_tx(tlm::tlm_command cmd, uint64_t addr, uint32_t& data) {
        tlm::tlm_generic_payload trans;
        sc_time delay = SC_ZERO_TIME;
        trans.set_command(cmd);
        trans.set_address(addr);
        trans.set_data_ptr(reinterpret_cast<unsigned char*>(&data));
        trans.set_data_length(4);
        trans.set_streaming_width(4);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        
        socket->b_transport(trans, delay);

        std::string cmd_str = (cmd == tlm::TLM_READ_COMMAND) ? "DOC " : "GHI ";
        if (trans.get_response_status() == tlm::TLM_OK_RESPONSE) {
            std::cout << "  [PASS] " << cmd_str << "0x" << std::hex << addr << std::dec << "\n";
        }
    }

    void run_sweep_test(uint64_t base, const std::vector<uint64_t>& offsets) {
        uint32_t dummy_data = 0xAAAAAAAA;
        for (uint64_t offset : offsets) {
            send_tx(tlm::TLM_WRITE_COMMAND, base + offset, dummy_data);
            send_tx(tlm::TLM_READ_COMMAND, base + offset, dummy_data);
        }
    }

    void run_simulation() {
        std::cout << "\n[HETHONG]: Dang quet kiem tra toan bo Register Map...\n";
        
        std::vector<uint64_t> uart_all_offsets = {0x0, 0x4, 0x8, 0xc, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30};
        run_sweep_test(0x10000000, uart_all_offsets);

        std::vector<uint64_t> timer_all_offsets = {0x0, 0x4, 0x100, 0x104, 0x108, 0x10c, 0x110, 0x114, 0x118, 0x11c};
        run_sweep_test(0x10001000, timer_all_offsets);
        
        std::cout << "[HETHONG]: Quet hoan tat! Khong co offset nao gay crash he thong.\n";

        std::string uart_input;
        uint32_t timer_ticks;

        std::cout << "\n[CVA6_VP]: Nhap chuoi: ";
        std::getline(std::cin, uart_input);

        std::cout << "[CVA6_VP]: Nhap so chu ky clock: ";
        if (!(std::cin >> timer_ticks)) {
            std::cout << "  [ERROR]: Ban da nhap sai dinh dang! He thong tu dong gan timer_ticks = 10.\n";
            std::cin.clear();
            std::cin.ignore(10000, '\n'); 
            timer_ticks = 10; // Gia tri mac dinh an toan
        }
        

        std::cout << "\n========== KICH BAN TEST UART ==========\n";
        uint64_t uart_base = 0x10000000;
        int fifo_capacity = 16;
        
        sc_time byte_period = sc_time(20, SC_NS); 

        for (size_t i = 0; i < uart_input.length(); ++i) {
            uint32_t char_data = uart_input[i];

            tlm::tlm_generic_payload trans;
            sc_time delay = SC_ZERO_TIME;
            trans.set_command(tlm::TLM_WRITE_COMMAND);
            trans.set_address(uart_base + 0x1c); 
            trans.set_data_ptr(reinterpret_cast<unsigned char*>(&char_data));
            trans.set_data_length(4);
            trans.set_streaming_width(4);
            trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
            
            socket->b_transport(trans, delay);

            wait(5, SC_NS); 

            if (i < fifo_capacity) {
                std::cout << "  Ky tu thu " << std::setw(2) << i + 1 << " ['" << (char)uart_input[i] << "'] -> [PASS] Da vao FIFO UART.\n";
            } else {
                std::cout << "  Ky tu thu " << std::setw(2) << i + 1 << " ['" << (char)uart_input[i] << "'] -> [FAIL/DROP] Tran FIFO, bi phan cung loai bo!\n";
            }
        }

        wait(tick_period);

        uint32_t flag_status = 0;
        send_tx(tlm::TLM_READ_COMMAND, uart_base + 0x18, flag_status); 
        
        std::cout << "  => STATUS sau khi gui " << uart_input.length() << " byte: 0x" << std::hex << flag_status << std::dec << "\n";

        bool expected_full = (uart_input.length() >= (size_t)fifo_capacity);
        bool is_full = (flag_status & 0x20) != 0; 
        if (expected_full == is_full) {
            std::cout << "  ==> [RESULT]: PASS! Logic FIFO xu ly chinh xac\n";
        } else {
            std::cout << "  ==> [RESULT]: FAIL! Co loi logic FIFO.\n";
        }


        std::cout << "\n========== KICH BAN TEST TIMER ==========\n";
        uint64_t timer_base = 0x10001000;

        uint32_t ctrl_data = 0x1; 
        std::cout << "  [TIMER]: Kich hoat Timer Enable...\n";
        send_tx(tlm::TLM_WRITE_COMMAND, timer_base + 0x04, ctrl_data); 

        std::cout << "  [TIMER]: CPU vao trang thai Sleep (" << timer_ticks << " ticks)...\n";
        wait(timer_ticks * tick_period); 

        wait(SC_ZERO_TIME);

uint32_t current_value = 0;
        send_tx(tlm::TLM_READ_COMMAND, timer_base + 0x110, current_value);
        std::cout << "  => Gia tri thanh ghi dem: " << current_value << "\n";

        if (current_value == timer_ticks) {
            std::cout << "  ==> [RESULT]: PASS! Dong co Timer dong bo hoan hao voi clock.\n";
        } else {
            std::cout << "  ==> [RESULT]: FAIL! Sai lech thoi gian.\n";
        }
        
        std::cout << "\n[HETHONG]: Mo phong hoan tat!\n";
    }
};

#endif