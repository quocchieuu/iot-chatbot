# Smart Home AI & IoT Chatbot System - Đồ Án Tốt Nghiệp

Hệ thống nhà thông minh tích hợp AI Chatbot/Voice, quản lý & điều khiển đa nền tảng qua Web/App, Firebase Realtime Database và các cảm biến/mô-đun phần cứng.

---

## 📺 Video Demo
Xem video demo hoạt động thực tế của hệ thống tại đây: 
👉 https://drive.google.com/file/d/1YtqSv-8gLF4dXTquEBfpiZtVmoEktL7Q/view?usp=sharing

## 🌟 Các Chức Năng Chính Của Hệ Thống

Dựa trên kiến trúc hệ thống, dự án bao gồm các phân hệ chức năng chính:

1. **Khởi tạo & Xác thực hệ thống:**
   - Khởi tạo tự động các mô-đun kết nối: Firebase, Voice AI, RFID, giao tiếp UART giữa các thiết bị.
   - Quản lý truy cập: Xác thực đăng nhập an toàn qua Web/App trước khi cho phép điều khiển.

2. **Các Mô-đun Điều Khiển & Giám Sát:**
   - 🎙️ **Hệ thống điều khiển bằng giọng nói (Voice Control / AI Worker):** Nhận diện lệnh giọng nói để xử lý và điều khiển thiết bị thông minh.
   - 🌐 **Hệ thống điều khiển qua Web / Mobile App:** Giao diện trực quan cho phép theo dõi trạng thái và đóng/mở thiết bị từ xa.
   - 🔑 **Khóa cửa thông minh RFID:** Quản lý ra vào an toàn bằng thẻ từ RFID.
   - 🛡️ **Hệ thống an ninh (Security Worker):** Giám sát, phát hiện cảnh báo đột nhập và bảo vệ ngôi nhà.
   - 🧠 **Hệ thống học thói quen người dùng:** Tự động hóa kịch bản dựa trên hành vi và lịch trình sử dụng.
   - 🌧️ **Hệ thống cảm biến mưa:** Tự động phát hiện thời tiết mưa để kích hoạt đóng cửa/mái che bảo vệ.

3. **Luồng Xử Lý & Phản Hồi:**
   - Xử lý lệnh trung tâm ➡️ Điều khiển thiết bị phần cứng thực tế.
   - Đồng bộ trạng thái theo thời gian thực (Realtime) lên **Firebase Database**.
   - Cập nhật tức thì giao diện điều khiển trên **Web/App**.
   - Phản hồi trạng thái cho người dùng bằng **Loa thông báo (Audio Feedback)**.

---

## 🛠️ Công Nghệ & Phần Cứng Sử Dụng

- **Phần cứng:** Raspberry Pi, ESP32, RFID Reader, Cảm biến mưa, Loa thông báo, Các cảm biến an ninh.
- **Phần mềm / Ngôn ngữ:** Python 3.11, C/C++, HTML/CSS/JavaScript (Dashboard Web/App).
- **Giao tiếp & Cơ sở dữ liệu:** Firebase Realtime Database, UART Communication.
