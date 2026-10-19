
#include <DHT.h>
#include <DHT_U.h>


// --- 1. KHAI BÁO CHÂN (GIỮ NGUYÊN) ---
#define DHT_K 4
#define DHT_N 5
#define GAS_SENSE 34
#define PIR_TOILET 35
#define PIR_TROM 36 
const int RGB_K[] = {12, 13, 14}; 
const int RGB_N[] = {25, 26, 27}; 
#define FAN_K 32
#define FAN_N 33
#define FAN_T 18
#define LED_T 19
#define LED_TRO 23
#define LED_GAS 15
#define BUZZER 2
#define BT_K 21
#define BT_N 22
#define BT_F_K 0   
#define BT_F_N 39
#define COI_KEU LOW
#define COI_TAT HIGH

DHT dhtK(DHT_K, DHT11);
DHT dhtN(DHT_N, DHT11);

const int levels[] = {0, 85, 170, 255}; 
int curL_K = 0, curL_N = 0, curF_K = 0, curF_N = 0;
bool last_K = LOW, last_N = LOW, last_FK = LOW, last_FN = LOW;
unsigned long lastSend = 0;
unsigned long lastToiletMove = 0; 
const long toiletTimeout = 3000; 
unsigned long lastBuzzerTick = 0;
bool buzzerState = false;
unsigned long lastPIRTime = 0; // Biến này để ở đầu file (ngoài loop)
const int alertTimeout = 5000; // Đợi 5 giây mới tắt báo động
int lastGas = -1; 
unsigned long lastGasChange = 0;
// --- BIẾN THÊM CHO NGỮ CẢNH (CẤM XÓA) ---
bool securityMode = false; 

void setup() {
  Serial.begin(115200); 
  Serial2.begin(115200, SERIAL_8N1, 17, 16); // Nối với Pi 4
  Serial2.setTimeout(10);
  int outputs[] = {FAN_K, FAN_N, FAN_T, LED_T, LED_TRO, LED_GAS, BUZZER};
  for(int p : outputs) pinMode(p, OUTPUT);
  for(int i=0; i<3; i++) { pinMode(RGB_K[i], OUTPUT); pinMode(RGB_N[i], OUTPUT); }

  digitalWrite(BUZZER, COI_TAT);
  digitalWrite(LED_TRO, LOW);
  digitalWrite(LED_GAS, LOW);

  pinMode(BT_K, INPUT); 
  pinMode(BT_N, INPUT);

  pinMode(BT_F_K, INPUT); 
  pinMode(BT_F_N, INPUT);
  pinMode(GAS_SENSE, INPUT); pinMode(PIR_TOILET, INPUT); pinMode(PIR_TROM, INPUT);

  dhtK.begin(); dhtN.begin();
}

int parseValue(String val) {
  val.toUpperCase();
  if (val == "ON" || val == "TRUE" || val == "OPEN") return 3;
  if (val == "OFF" || val == "FALSE" || val == "CLOSE") return 0;
  return val.toInt();
}

// Hàm cập nhật màu sắc đèn RGB (Thêm màu sắc cho ngữ cảnh)
void updateLightColor(const int pins[], int level, int r, int g, int b) {
  if (level == 0) {
    for(int i=0; i<3; i++) analogWrite(pins[i], 0);
  } else {
    // r, g, b nhận giá trị 0 hoặc 1 để bật/tắt kênh màu
    analogWrite(pins[0], r * levels[level]);
    analogWrite(pins[1], g * levels[level]);
    analogWrite(pins[2], b * levels[level]);
  }
}

// Giữ nguyên hàm cũ của ông để không lỗi các chỗ khác
void updateLight(const int pins[], int level) {
  int val = levels[constrain(level, 0, 3)];
  for(int i=0; i<3; i++) analogWrite(pins[i], val); // Mặc định là màu trắng (3 kênh bằng nhau)
}
// Thêm 2 biến này vào trên đầu (ngoài loop) để chặn gửi trùng
int lastTrom = -1;

// --- TRONG LOOP ---

void loop() {
  // --- 1. NHẬN LỆNH TỪ PI (BAO GỒM NGỮ CẢNH) ---
  while (Serial2.available() > 0) {
    String cmd = Serial2.readStringUntil('\n');
    cmd.trim();
    int colonIdx = cmd.indexOf(':');
    if (colonIdx != -1) {
      String key = cmd.substring(0, colonIdx);
      String valStr = cmd.substring(colonIdx + 1);
      int val = parseValue(valStr);

      /// --- LOGIC NGỮ CẢNH THEO YÊU CẦU ---
      if (key == "SecurityMode") { 
          securityMode = (val > 0); 
      }
      
      // Xử lý nốt "Khach" (Cả số và Chữ ngữ cảnh từ Web gửi xuống)
      else if (key == "Khach") {
          if (valStr == "MOVIE") {
              updateLightColor(RGB_K, 2, 1, 1, 0); // Màu vàng phim
              analogWrite(FAN_K, levels[3]);       // Quạt khách mức 3
          } 
          else if (valStr == "PARTY") {
              updateLightColor(RGB_K, 3, 1, 0, 1); // Màu hồng tiệc
              analogWrite(FAN_K, levels[3]); 
          } 
          else {
              curL_K = val; 
              updateLight(RGB_K, curL_K); // Bật trắng bình thường nếu là số 0-3
          }
      }

      // Xử lý nốt "Ngu" (Ngữ cảnh SLEEP)
      else if (key == "Ngu") {
          if (valStr == "SLEEP") {
              updateLightColor(RGB_N, 1, 1, 1, 0); // Đèn ngủ màu vàng
              analogWrite(FAN_N, levels[2]);       // Quạt ngủ mức 2
              securityMode = true;                 // Bật an ninh luôn
              updateLightColor(RGB_K, 0, 0, 0, 0); // Tắt đèn khách cho chắc
          } 
          else {
              curL_N = val; 
              updateLight(RGB_N, curL_N);
          }
      }

      // Giữ lại các cái này cho Voice (Vì Voice gửi thẳng lệnh SLEEP/MOVIE/PARTY)
      else if (key == "SLEEP") {
          updateLightColor(RGB_N, 1, 1, 1, 0);
          analogWrite(FAN_N, levels[2]);
          securityMode = true;
          updateLightColor(RGB_K, 0, 0, 0, 0);
      }
      else if (key == "MOVIE") {
          updateLightColor(RGB_K, 2, 1, 1, 0);
          analogWrite(FAN_K, levels[3]);
      }
      else if (key == "PARTY") {
          updateLightColor(RGB_K, 3, 1, 0, 1);
          analogWrite(FAN_K, levels[3]);
      }

      // Điều khiển cơ bản còn lại
      else if (key == "Fan_Khach") { curF_K = val; analogWrite(FAN_K, levels[curF_K]); }
      else if (key == "Fan_Ngu") { curF_N = val; analogWrite(FAN_N, levels[curF_N]); }
      else if (key == "Toilet") { digitalWrite(LED_T, val > 0); digitalWrite(FAN_T, val > 0); }

      // Điều khiển cơ bản (Giữ nguyên)
      else if (key == "Fan_Khach") { curF_K = val; analogWrite(FAN_K, levels[curF_K]); }
      else if (key == "Fan_Ngu") { curF_N = val; analogWrite(FAN_N, levels[curF_N]); }
      else if (key == "Khach") { curL_K = val; updateLight(RGB_K, curL_K); }
      else if (key == "Ngu") { curL_N = val; updateLight(RGB_N, curL_N); }
      else if (key == "Toilet") { digitalWrite(LED_T, val > 0); digitalWrite(FAN_T, val > 0); }
    }
  }

// --- TRONG LOOP ---

// Mặc định là tắt còi
int buzzerMode = 0; // 0: Tắt, 1: Gas (Kêu nhanh), 2: Trộm (Kêu chậm)

// 1. Kiểm tra Gas (Ưu tiên cao nhất)
if (digitalRead(GAS_SENSE) == LOW) {
    buzzerMode = 1; 
} 
// 2. Kiểm tra Trộm (Nếu không có Gas mới xét đến Trộm)
else if (securityMode && digitalRead(PIR_TROM) == HIGH) {
    buzzerMode = 2;
}

// --- KHỐI ĐIỀU KHIỂN CÒI THEO NHỊP ---
if (buzzerMode == 1) { // KIỂU GAS: Tít tít tít liên tục (Rất nhanh)
    if (millis() - lastBuzzerTick > 100) { // Cứ 100ms đảo trạng thái
        buzzerState = !buzzerState;
        digitalWrite(BUZZER, buzzerState ? COI_KEU : COI_TAT);
        lastBuzzerTick = millis();
    }
} 
else if (buzzerMode == 2) { // KIỂU TRỘM: Tít... tít... tít... (Chậm hơn)
    if (millis() - lastBuzzerTick > 500) { // Cứ 500ms đảo trạng thái
        buzzerState = !buzzerState;
        digitalWrite(BUZZER, buzzerState ? COI_KEU : COI_TAT);
        lastBuzzerTick = millis();
    }
} 
else { // TẮT CÒI
    digitalWrite(BUZZER, COI_TAT);
    buzzerState = false;
}

// --- 2. HỆ THỐNG AN NINH (PIR TRỘM) ---
// Chỉ xử lý nếu chế độ An ninh đang BẬT (securityMode == true)
if (securityMode == true) { 
    int rawTrom = digitalRead(PIR_TROM);
    if (rawTrom == HIGH) {
        lastPIRTime = millis(); // Cứ thấy người là cập nhật mốc thời gian
    }
    // Nếu có người (HIGH)
    if (millis() - lastPIRTime < alertTimeout) {
        if (lastTrom != 1) { // Chỉ gửi khi bắt đầu phát hiện
            digitalWrite(LED_TRO, HIGH);
            digitalWrite(BUZZER, COI_KEU);
            Serial2.println("UPDATE_TROM:1");
            lastTrom = 1;
        }
    } 
    // Nếu không có người (LOW)
    else {
        if (lastTrom != 0) {
            digitalWrite(LED_TRO, LOW);
            // Chỉ tắt còi nếu Gas cũng đang an toàn
            if (digitalRead(GAS_SENSE) == HIGH) digitalWrite(BUZZER, COI_TAT);
            Serial2.println("UPDATE_TROM:0");
            lastTrom = 0;
        }
    }
} else {
    // Nếu An ninh OFF: Đảm bảo tắt hết báo động trộm
    if (lastTrom != 0) {
        digitalWrite(LED_TRO, LOW);
        if (digitalRead(GAS_SENSE) == HIGH) digitalWrite(BUZZER, COI_TAT);
        Serial2.println("UPDATE_TROM:0");
        lastTrom = 0;
    }
}

// --- 3. HỆ THỐNG CẢNH BÁO GAS (CHỈ SỬA PHẦN NÀY) ---
int rawGas = digitalRead(GAS_SENSE); // Đọc trực tiếp từ chân 34
int curGas = (rawGas == LOW) ? 1 : 0; // LOW là có gas (LED sáng), HIGH là an toàn (LED tắt)

if (curGas != lastGas) {
  // Nếu trạng thái thay đổi, đợi 200ms để xác nhận (chống nhiễu tín hiệu nhảy)
  if (millis() - lastGasChange > 200) { 
    if (curGas == 1) {
        digitalWrite(LED_GAS, HIGH);
        // Không dùng digitalWrite BUZZER ở đây vì đã có khối điều khiển còi theo nhịp ở trên lo rồi
        Serial2.println("UPDATE_GAS:DANGER"); 
        lastGas = 1;
    } else {
        digitalWrite(LED_GAS, LOW);
        Serial2.println("UPDATE_GAS:SAFE");
        lastGas = 0;
    }
  }
} else {
  // Nếu tín hiệu đang nhảy liên tục, reset mốc thời gian
  lastGasChange = millis(); 
}
  // --- 4. CÁC HÀM CŨ (GIỮ NGUYÊN 100%) ---
  handleButton(BT_K, last_K, curL_K, "Khach", true); 
  handleButton(BT_N, last_N, curL_N, "Ngu", true); 
  handleButton(BT_F_K, last_FK, curF_K, "Fan_Khach", false);
  handleButton(BT_F_N, last_FN, curF_N, "Fan_Ngu", false);

 // --- LOGIC TOILET TRÊN ESP32 ---
  if (digitalRead(PIR_TOILET) == HIGH) {
    if (digitalRead(LED_T) == LOW) { // Chỉ gửi khi bắt đầu nhận người
        Serial2.println("UPDATE_TOILET_L:1"); // L là Light
        Serial2.println("UPDATE_TOILET_F:1"); // F là Fan
    }
    lastToiletMove = millis();
    digitalWrite(LED_T, HIGH); 
    digitalWrite(FAN_T, HIGH);
  } 
  else if (millis() - lastToiletMove > 5000) { // Đợi 10s không có người thì tắt
    if (digitalRead(LED_T) == HIGH) { 
        digitalWrite(LED_T, LOW); 
        digitalWrite(FAN_T, LOW);
        Serial2.println("UPDATE_TOILET_L:0");
        Serial2.println("UPDATE_TOILET_F:0");
    }
  }

  if (millis() - lastSend > 10000) {

    float tk = dhtK.readTemperature();
    float hk = dhtK.readHumidity();

    float tn = dhtN.readTemperature();
    float hn = dhtN.readHumidity();

    if (!isnan(tk)) {

        Serial2.println(
            "SENSOR_Khach_Temp:" +
            String(tk)
        );

        Serial2.println(
            "SENSOR_Khach_Humi:" +
            String(hk)
        );
    }

    if (!isnan(tn)) {

        Serial2.println(
            "SENSOR_Ngu_Temp:" +
            String(tn)
        );

        Serial2.println(
            "SENSOR_Ngu_Humi:" +
            String(hn)
        );
    }

    lastSend = millis();
}

yield();
}

void handleButton(
    int pin,
    bool &last,
    int &level,
    String label,
    bool isLED
  ) {

    bool cur = digitalRead(pin);
    delay(5);

    cur = digitalRead(pin);

    // ==========================
    // NHẤN = LOW
    // ==========================

    if (cur == HIGH && last == LOW) {

        delay(30);

        if (digitalRead(pin) != HIGH)
            return;

        // ======================
        // TĂNG LEVEL
        // ======================

        level = (level + 1) % 4;

        // ======================
        // LED
        // ======================

        if(isLED) {

            if (label == "Khach")
                updateLight(RGB_K, level);

            else
                updateLight(RGB_N, level);

        }

        // ======================
        // FAN
        // ======================

        else {

            if (label == "Fan_Khach")
                analogWrite(
                    FAN_K,
                    levels[level]
                );

            else
                analogWrite(
                    FAN_N,
                    levels[level]
                );
        }

        // ======================
// UART
// ======================

String uartMsg =
    "UPDATE_" +
    label +
    ":" +
    String(level);

// GỬI QUA PI

Serial2.println(uartMsg);


// DEBUG PC

Serial.println(uartMsg);

// ======================
// ĐỢI NHẢ NÚT
// ======================

delay(20);
}
last = cur;}