# CVA6 Virtual Platform - Dummy CPU 

Repo này là nền tảng trong việc xây dựng Virtual Platform (VP) cho hệ thống SoC dựa trên chuẩn **SystemC/TLM-2.0 Loosely-Timed**. 

Trong giai đoạn này, lõi xử lý sử dụng một **Dummy CPU** đóng vai trò Initiator Master để kiểm tra lưới định tuyến, sơ đồ ánh xạ bộ nhớ  và cơ chế bắt tín hiệu ngắt phần cứng bất đồng bộ (WFI/IRQ) trước khi tích hợp lõi xử lý thực tế.

---

## 🛠️ Yêu cầu Hệ thống & Công cụ
Để cấu hình, biên dịch và vận hành mô hình mô phỏng, môi trường Linux/WSL của bạn cần được cài đặt sẵn:
* **Hệ điều hành:** Linux (Ubuntu) hoặc WSL trên Windows.
* **Công cụ biên dịch:** `CMake >= 3.21`, `GCC/G++` (Hỗ trợ chuẩn C++17), `Make`.
* **Thư viện core:** `SystemC 2.3.4` (Đã cấu hình đường dẫn biến môi trường hoặc cài đặt sẵn tại hệ thống).

*Lưu ý: Do lõi xử lý là CPU ảo tự phát sinh giao dịch trực tiếp từ mã nguồn C++, hệ thống **hoàn toàn không cần cài đặt bộ dịch chéo (cross-compiler) của RISC-V** ở nhánh này.*

---

## 🗺️ Memory Map
Mạng định tuyến Bus giải mã địa chỉ  dựa trên các vùng không gian ô nhớ cố định:
* **UART Target (Cổng xuất dữ liệu màn hình TX):** Địa chỉ vùng chứa `0x10000000` - `0x10000FFF`
  * Thanh ghi dữ liệu tương tác: Offset `0x1C` (Địa chỉ vật lý: `0x1000001C`)
* **Timer Target (Bộ đếm nhịp thời gian hệ thống):** Địa chỉ vùng chứa `0x10030000` - `0x10030FFF`
  * Thanh ghi điều khiển chạy (`TIMER_CTRL`): Offset `0x04` (Địa chỉ vật lý: `0x10030004`)
  * Thanh ghi đặt mốc so sánh ngắt (`TIMER_COMPARE`): Offset `0x118` (Địa chỉ vật lý: `0x10030118`)

---

## 📂 Sơ đồ Tổ chức Cấu trúc Dự án (Repository Structure)

Mã nguồn dự án được phân tách cấu trúc rõ ràng thành các thư mục và tệp tin chuyên trách:


```text
. (Thư mục gốc)
├── CMakeLists.txt
├── main.cpp
├── my_vp.exe
├── 📁 docs/
│   └── simulation_result.png
│
├── 📁 .vscode/            
│   └── settings.json      
│
├── 📁 IPs/                
│   ├── simple_router.h    
│   ├── uart_target.h       
│   └── timer_target.h     
│
└── 📁 CPU/                
    └── cva6_initiator.h   
```


## 🚀 Hướng dẫn Biên dịch và Khởi chạy từ Thư mục gốc

Bạn hãy mở Terminal ngay tại thư mục gốc ngoài cùng của dự án (`my-cva6-custom-dummy-cpu`) và chạy chuỗi lệnh tự động hóa siêu ngắn gọn sau:

### Thực thi thủ công từng lệnh CMake tiêu chuẩn
```bash
# 1. Tạo thư mục build và cấu hình dự án
mkdir -p build && cd build
cmake ..

# 2. Biên dịch phần cứng ảo (Sản phẩm tự động đẩy ngược ra thư mục cha ngoài cùng)
make -j4
cd ..

# 3. Khởi chạy mô phỏng
stty -icanon -echo && ./my_vp.exe; stty sane
```
---

## 📊 Kết quả Mô phỏng Kỳ vọng 

Khi khởi chạy, mô phỏng sẽ vận hành tuần tự, chính xác theo dòng thời gian tuyến tính của hạt nhân SystemC và tự động kết thúc an toàn khi Dummy CPU hoàn tất vòng lặp phát sinh giao dịch.

Các gói tin payload sẽ được Router định tuyến chính xác tới các ngoại vi UART và Timer mà không xảy ra bất kỳ hiện tượng tranh chấp luồng, rách ký tự hay lỗi tràn bộ nhớ (Segmentation Fault / Out-of-bounds).
Hệ thống sau khi gọi lệnh `sc_stop()` sẽ đóng luồng an toàn và nhả ra câu chốt hạ nguyên bản của SystemC kernel:

```text
Starting simulation...
DONE
[CPU Ao] Cấu hình Timer...
[Timer] Nhận cấu hình từ CPU - Offset: 0x118 | Giá trị: 500000
[Timer] Nhận cấu hình từ CPU - Offset: 0x4 | Giá trị: 1
[CPU Ao] Chuyển sang chế độ WFI (Chờ ngắt)...
[Timer] Bắt đầu đếm 500000 ns...
[Timer] Ngắt Timer | Time: 502 us
IRQ!

Info: /OSCI/SystemC: Simulation stopped by user.
Simulation finished.
```

```


    
