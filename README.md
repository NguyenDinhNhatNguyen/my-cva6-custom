# CVA6 RISC-V Virtual Platform 

Hệ thống Virtual Platform (VP) tích hợp mô hình **Instruction Set Simulator (ISS)** cho lõi RISC-V (`RV32IM_Zicsr`), hỗ trợ thực thi Bare-metal Firmware (C/C++), quét bàn phím thời gian thực và xử lý ngắt (Machine External Interrupt - Timer IRQ Line 11).

---

## 🛠️ Yêu cầu Hệ thống & Trình dịch chéo
* **Hệ điều hành:** Linux (Ubuntu) hoặc WSL.
* **Công cụ:** `CMake >= 3.21`, `GCC/G++`, `Make`, `SystemC 2.3.4`.
* **Trình dịch chéo:** `gcc-riscv64-unknown-elf`
```bash
sudo apt update && sudo apt install gcc-riscv64-unknown-elf
```

---

## 🗺️ Bản đồ Địa chỉ (Memory Map)
* **RAM (2MB):** `0x80000000` - `0x801FFFFF`
* **UART Target:** `0x10000000` - `0x10000FFF` (Data: `0x1000001C`)
* **Timer Target:** `0x10030000` - `0x10030FFF` (Ctrl: `0x10030004`, Compare: `0x10030118`)

---

## 📂 Cấu trúc Dự án
* `main.cpp` : Top-level netlist, khởi tạo hệ thống và cấu hình Terminal.
* `CMakeLists.txt` : Script cấu hình CMake.
* `hw/` : Ngoại vi phần cứng (`simple_router.h`, `ram_target.h`, `uart_target.h`, `timer_target.h`).
* `fw/` : Mã nguồn firmware (`main.c`, `link.ld`).
* `cpu_models/` : Mô hình lõi xử lý và ELF Loader.
* `third_party/` : Thư viện ngoài (`riscv-vp`, `SoftFloat`).

---

## 🚀 Hướng dẫn Khởi chạy

1. **Biên dịch Firmware (.elf):**
```bash
riscv64-unknown-elf-gcc -O2 -ffreestanding -nostdlib -march=rv32im_zicsr -mabi=ilp32 -T fw/link.ld fw/main.c -o firmware.elf -Wl,--no-warn-rwx-segments
```

2. **Biên dịch SystemC:**
```bash
mkdir -p build && cd build && cmake .. && make -j4 && cd ..
```

3. **Khởi chạy:**
```bash
stty -icanon -echo && ./my_vp.exe firmware.elf; stty sane
```

---

## 📊 Kết quả Mô phỏng 

Khi khởi chạy, mô phỏng sẽ nạp tệp `firmware.elf` vào RAM và thực thi các chỉ thị mã máy một cách tuần tự. Hệ thống hỗ trợ tương tác bàn phím thời gian thực và xử lý mạch ngắt bất đồng bộ một cách chuẩn xác.

Log hiển thị nguyên bản trên cửa sổ Terminal khi vận hành:

```text
        SystemC 2.3.4-Accellera --- Sep 26 2026 02:33:29
        Copyright (c) 1996-2022 by all Contributors,
        ALL RIGHTS RESERVED
Starting simulation...

===========================================
  FIRMWARE TEST: UART (IO) + TIMER (IRQ) 
===========================================

>> Vui lòng nhập tên của bạn: Nguyên

Xin chào, Nguyên

Kích hoạt Timer test ngắt 1 lần...


>>> [IRQ] CPU nhận được ngắt từ Timer Hardware! <<<
>> Hệ thống đã xử lý xong ngắt.
```
