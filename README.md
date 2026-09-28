# CVA6 Virtual Platform - Register Map & Offset Verification (test-offset branch)

Nhánh độc lập `test-offset`, tập trung vào việc xác thực toàn diện không gian địa chỉ vật lý, kiểm tra tính toàn vẹn của mạng lưới định tuyến và chạy các kịch bản Testbench (UART FIFO, Synchronous Timer Clock) dựa trên cấu trúc tập thanh ghi chuẩn OpenTitan của khối `uart` và `rv_timer`.

---

## Yêu cầu Hệ thống & Công cụ
Để biên dịch và vận hành mô hình phần cứng giả lập, hệ thống của bạn cần cài đặt sẵn:
* **Hệ điều hành:** Linux (Ubuntu) hoặc môi trường WSL trên Windows.
* **Công cụ xây dựng:** `CMake >= 3.21`, trình biên dịch `GCC/G++` (Hỗ trợ C++17), và `Make`.
* **Thư viện core:** `SystemC 2.3.4` (Yêu cầu cấu hình chính xác biến môi trường `$SYSTEMC_HOME`).

---

## Memory-Mapped Registers

Hệ thống SoC định tuyến và phân bổ không gian địa chỉ cho các thiết bị ngoại vi dựa trên hai vùng Base Address cố định:

### 1. Khối Ngoại vi UART (`uart`)
* **Base Address:** `0x10000000`
* **Dải Register Map:**

| Tên Thanh Ghi | Offset | Độ Dài (Byte) | Mô Tả Chức Năng |
| :--- | :---: | :---: | :--- |
| `uart.INTR_STATE` | `0x00` | 4 | Interrupt State Register (Trạng thái ngắt) |
| `uart.INTR_ENABLE` | `0x04` | 4 | Interrupt Enable Register (Cho phép ngắt) |
| `uart.INTR_TEST` | `0x08` | 4 | Interrupt Test Register (Kiểm tra ngắt bằng phần mềm) |
| `uart.ALERT_TEST` | `0x0C` | 4 | Alert Test Register (Kiểm tra cảnh báo lỗi) |
| `uart.CTRL` | `0x10` | 4 | UART Control Register (Thanh ghi điều khiển UART) |
| `uart.STATUS` | `0x14` | 4 | UART Live Status Register (Trạng thái hoạt động thời gian thực) |
| `uart.RDATA` | `0x18` | 4 | UART Read Data (Thanh ghi đọc dữ liệu nhận về) |
| `uart.WDATAA` | `0x1C` | 4 | UART Write Data (Thanh ghi ghi dữ liệu truyền đi) |
| `uart.FIFO_CTRL` | `0x20` | 4 | UART FIFO Control Register (Điều khiển hàng đợi FIFO) |
| `uart.FIFO_STATUS` | `0x24` | 4 | UART FIFO Status Register (Trạng thái bộ đệm FIFO) |
| `uart.OVRD` | `0x28` | 4 | TX pin override control (SW trực tiếp điều khiển chân TX) |
| `uart.VAL` | `0x2C` | 4 | UART oversampled values (Giá trị lấy mẫu tín hiệu) |
| `uart.TIMEOUT_CTRL` | `0x30` | 4 | UART RX timeout control (Điều khiển thời gian chờ RX) |

### 2. Khối Ngoại vi Bộ định thời (`rv_timer`)
* **Base Address:** `0x10030000`
* **Dải Register Map:**

| Tên Thanh Ghi | Offset | Độ Dài (Byte) | Mô Tả Chức Năng |
| :--- | :---: | :---: | :--- |
| `rv_timer.ALERT_TEST` | `0x00` | 4 | Alert Test Register (Kiểm tra cảnh báo lỗi) |
| `rv_timer.CTRL` | `0x04` | 4 | Control register (Thanh ghi điều khiển bật/tắt Timer) |
| `rv_timer.INTR_ENABLE0` | `0x100` | 4 | Interrupt Enable for Hart 0 (Cho phép ngắt lõi xử lý 0) |
| `rv_timer.INTR_STATE0` | `0x104` | 4 | Interrupt Status for Hart 0 (Trạng thái ngắt lõi xử lý 0) |
| `rv_timer.INTR_TEST0` | `0x108` | 4 | Interrupt test register for Hart 0 (Kiểm tra ngắt Hart 0) |
| `rv_timer.CFG0` | `0x10C` | 4 | Configuration for Hart 0 (Cấu hình bộ đếm cho Hart 0) |
| `rv_timer.TIMER_V_LOWER0` | `0x110` | 4 | Timer value Lower (Giá trị bộ đếm hiện tại - 32-bit thấp) |
| `rv_timer.TIMER_V_UPPER0` | `0x114` | 4 | Timer value Upper (Giá trị bộ đếm hiện tại - 32-bit cao) |
| `rv_timer.COMPARE_LOWER0_0` | `0x118` | 4 | Timer value Lower (Mốc so sánh kích hoạt ngắt - 32-bit thấp) |
| `rv_timer.COMPARE_UPPER0_0` | `0x11C` | 4 | Timer value Upper (Mốc so sánh kích hoạt ngắt - 32-bit cao) |

---

## Sơ đồ Tổ chức Cấu trúc Dự án 

Mã nguồn trên nhánh `test-offset` được tổ chức phẳng gọn gàng và phân tách rõ ràng thành các thư mục chuyên trách:

* `main.cpp` : File C++ kết nối mạch cấp cao nhất (Top-level netlist), liên kết Sockets phần cứng.
* `CMakeLists.txt` : Script cấu hình CMake điều khiển quét thư viện và biên dịch dự án.
* `Makefile` : Tập hợp các phím tắt tự động hóa chuỗi lệnh dọn dẹp, build và chạy nhanh.
* `my_vp.exe` : [Sản phẩm đầu ra] Chương trình thực thi giả lập sau khi biên dịch thành công.
* `simulation_result.png` : [Tài liệu] Ảnh chụp màn hình chứng thực log quét toàn bộ Register Map thực tế.

```text
. (Thư mục gốc phẳng)
├── CMakeLists.txt
├── main.cpp
├── my_vp.exe
├── Sim_Result.png
│
├── 📁 .vscode/             
│   └── c_cpp_properties.json 
│
├── 📁 IPs/                 
│   ├── simple_router.h    
│   ├── uart_target.h      
│   └── timer_target.h     
│
└── 📁 CPU/                
    └── cva6_initiator.h    
```

---

## Hướng dẫn Biên dịch và Khởi chạy

Bạn mở cửa sổ Terminal ngay tại thư mục gốc ngoài cùng của dự án và thực hiện theo một trong hai cách:

### Thực thi thủ công bằng lệnh CMake tiêu chuẩn
```bash
# 1. Tạo không gian bộ đệm sạch
mkdir -p build && cd build

# 2. Quét cấu hình hệ thống với biến môi trường $SYSTEMC_HOME
cmake ..

# 3. Biên dịch lõi cứng (Sản phẩm tự động đẩy ngược ra thư mục cha)
make -j4
cd ..

# 4. Khởi chạy mô phỏng
stty -icanon -echo && ./my_vp.exe; stty sane
```

---

## Kết quả Mô phỏng Kỳ vọng 

Khi vận hành, mô hình kiểm thử sẽ tự động thực thi chuỗi kịch bản nghiệm thu tuyến tính:
1. **Quét toàn bộ Register Map:** CPU ảo thực hiện thao tác ghi rồi đọc lại tuần tự qua từng thanh ghi đơn lẻ của `uart` (từ `0x00` đến `0x30`) và `rv_timer` (từ `0x00` đến `0x11C`). Hệ thống phải đảm bảo không có bất kỳ offset nào gây lỗi tràn vùng nhớ hoặc crash Core.
2. **Xác thực logic FIFO UART:** Nhập chuỗi ký tự mẫu (Ví dụ: `FPGA à`), mạch cứng bóc tách và đẩy tuần tự từng byte vào bộ đệm FIFO, kiểm tra trạng thái thanh ghi `uart.STATUS` hoặc `uart.FIFO_STATUS` để đảm bảo trả về `0x0` khi hoàn tất.
3. **Xác thực động cơ Timer:** CPU ảo nạp giá trị kích hoạt qua thanh ghi `rv_timer.CTRL` và đặt số chu kỳ clock mô phỏng (Ví dụ: `5 ticks`) qua thanh ghi so sánh `rv_timer.COMPARE_LOWER0_0`. Timer phần cứng đếm đồng bộ với xung clock hệ thống, CPU ngủ (Sleep) và thức dậy đọc chính xác giá trị tích lũy từ `rv_timer.TIMER_V_LOWER0`.

Log hiển thị nguyên bản trên màn hình Console (dựa trên bài test gần nhất):
```text
SystemC 2.3.4-Accellera --- Sep 26 2026 02:33:29
Copyright (c) 1996-2022 by all Contributors,
ALL RIGHTS RESERVED

[HETHONG]: Dang quet kiem tra toan bo Register Map...
[PASS] DOC 0x10000000
[PASS] GHI 0x10000000
...
[PASS] GHI 0x1000111c
[PASS] DOC 0x1000111c
[HETHONG]: Quet hoan tat! Khong co offset nao gay crash he thong.

[CVA6_VP]: Nhap chu: FPGA à
[CVA6_VP]: Nhap so chu ky clock: 5

========== KICH BAN TEST UART ==========
Ky tu thu 1 ['F'] -> [PASS] Da vao FIFO UART.
...
Ky tu thu 9 [' '] -> [PASS] Da vao FIFO UART.
[PASS] DOC 0x10000018
=> STATUS sau khi gui 9 byte: 0x0
=> [RESULT]: PASS! Logic FIFO xu ly chinh xac

========== KICH BAN TEST TIMER ==========
[TIMER]: Kich hoat Timer Enable...
[PASS] GHI 0x10001004
[TIMER]: CPU vao trang thai Sleep (5 ticks)...
[PASS] DOC 0x10001110
=> Gia tri thanh ghi dem: 5
=> [RESULT]: PASS! Dong co Timer dong bo hoan hao voi clock.

[HETHONG]: Mo phong hoan tat!
```
