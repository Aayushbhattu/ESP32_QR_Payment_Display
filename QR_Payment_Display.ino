/********************************************************************************
 * Project: Dynamic QR Payment Display (FINAL STABLE VERSION)
 * Board:   ESP32-WROOM-32
 * Display: ILI9341 2.4" 8-bit Parallel (Arduino_GFX)
 * QR Lib:  qrcodegen (Nayuki) — qrcodegen.hpp + qrcodegen.cpp in this folder
 ********************************************************************************/

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_GFX_Library.h>
#include "qrcodegen.hpp"

using namespace qrcodegen;

/* ─────────────────── WIFI ─────────────────── */
const char* ssid     = "bhattu_fpkhr_2";
const char* password = "CLEB4FF26B";

/* ─────────────────── COLORS (RGB565) ─────────────────── */
#define BLACK   0x0000
#define WHITE   0xFFFF
#define YELLOW  0xFFE0
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0

/* ─────────────────── TFT CONTROL PINS ─────────────────── */
#define TFT_CS   5
#define TFT_DC   17
#define TFT_WR   16
#define TFT_RD   4
#define TFT_RST  23

/* ─────────────────── TFT DATA PINS ─────────────────── */
#define TFT_D0   12
#define TFT_D1   13
#define TFT_D2   26
#define TFT_D3   25
#define TFT_D4   33
#define TFT_D5   32
#define TFT_D6   21
#define TFT_D7   22

/* ─────────────────── BUZZER ─────────────────── */
#define BUZZER   27

/* ─────────────────── DISPLAY ─────────────────── */
Arduino_DataBus *bus = new Arduino_ESP32PAR8(
    TFT_DC, TFT_CS, TFT_WR, TFT_RD,
    TFT_D0, TFT_D1, TFT_D2, TFT_D3,
    TFT_D4, TFT_D5, TFT_D6, TFT_D7
);
Arduino_GFX *gfx = new Arduino_ILI9341(bus, TFT_RST);

/* ─────────────────── SERVER ─────────────────── */
AsyncWebServer server(80);

/* ─────────────────── STATE ─────────────────── */
enum ScreenState {
  SCREEN_WELCOME,
  SCREEN_BILL,
  SCREEN_QR,
  SCREEN_PAYMENT
};
ScreenState screen = SCREEN_WELCOME;

/* ─────────────────── DATA ─────────────────── */
String billId        = "BILL-001";
float  amount        = 0;
String qrData        = "https://example.com";
String paymentStatus = "PENDING";

/* ─────────────────── SERIAL BUFFER ─────────────────── */
String serialBuffer = "";

/* ─────────────────── BUZZER ─────────────────── */
void beep() {
  digitalWrite(BUZZER, HIGH);
  delay(40);
  digitalWrite(BUZZER, LOW);
}

/* ─────────────────── CENTER TEXT ─────────────────── */
void centerText(String text, int y, int size, uint16_t color) {
  gfx->setTextSize(size);
  gfx->setTextColor(color);
  int16_t  x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(text.c_str(), 0, y, &x1, &y1, &w, &h);
  gfx->setCursor((gfx->width() - (int16_t)w) / 2, y);
  gfx->print(text);
}

/* ─────────────────── SCREENS ─────────────────── */

void showWelcome() {
  gfx->fillScreen(BLACK);
  centerText("WELCOME",          60,  3, WHITE);
  centerText("ESP32 POS SYSTEM", 110, 2, WHITE);
}

void showBill() {
  gfx->fillScreen(BLACK);
  gfx->setTextColor(YELLOW);
  gfx->setTextSize(2);
  gfx->setCursor(10, 20);
  gfx->println("BILL INFO");
  gfx->setTextColor(WHITE);
  gfx->setCursor(10, 60);
  gfx->println("ID: " + billId);
  gfx->setCursor(10, 90);
  gfx->println("Amount: Rs." + String(amount, 2));
  gfx->setCursor(10, 120);
  gfx->println("Status: " + paymentStatus);
}

void showQR(String data) {
  gfx->fillScreen(WHITE);

  QrCode qr = QrCode::encodeText(data.c_str(), QrCode::Ecc::MEDIUM);
  int qrSize = qr.getSize();

  int displayW = gfx->width();
  int displayH = gfx->height();
  int maxDim   = min(displayW, displayH) - 16;
  int scale    = maxDim / qrSize;
  if (scale < 1) scale = 1;

  int totalPx = qrSize * scale;
  int offsetX = (displayW - totalPx) / 2;
  int offsetY = (displayH - totalPx) / 2 - 10;

  for (int y = 0; y < qrSize; y++) {
    for (int x = 0; x < qrSize; x++) {
      uint16_t color = qr.getModule(x, y) ? BLACK : WHITE;
      gfx->fillRect(offsetX + x * scale, offsetY + y * scale, scale, scale, color);
    }
  }

  int labelY = offsetY + totalPx + 6;
  centerText("Scan to Pay", labelY, 1, BLACK);
}

void showPayment() {
  uint16_t bg = (paymentStatus == "PAID") ? GREEN : RED;
  gfx->fillScreen(bg);
  centerText("PAYMENT",     80,  3, WHITE);
  centerText(paymentStatus, 140, 2, WHITE);
}

/* ─────────────────── SCREEN UPDATE ─────────────────── */

void updateScreen() {
  switch (screen) {
    case SCREEN_WELCOME: showWelcome();  break;
    case SCREEN_BILL:    showBill();     break;
    case SCREEN_QR:      showQR(qrData); break;
    case SCREEN_PAYMENT: showPayment();  break;
  }
}

/* ─────────────────── COMMAND HANDLER ─────────────────── */

void handleCommand(String cmd) {
  cmd.trim();
  if (cmd.length() == 0) return;

  if      (cmd == "welcome")          screen = SCREEN_WELCOME;
  else if (cmd == "bill")             screen = SCREEN_BILL;
  else if (cmd == "paid") {
    paymentStatus = "PAID";
    screen = SCREEN_PAYMENT;
  }
  else if (cmd == "pending") {
    paymentStatus = "PENDING";
    screen = SCREEN_BILL;
  }
  else if (cmd.startsWith("id:"))     billId = cmd.substring(3);
  else if (cmd.startsWith("amount:")) amount = cmd.substring(7).toFloat();
  else if (cmd.startsWith("qr:")) {
    qrData = cmd.substring(3);
    screen = SCREEN_QR;
  }

  beep();
  updateScreen();
  Serial.println("CMD: " + cmd + " | IP: " + WiFi.localIP().toString());
}

/* ─────────────────── REST API ─────────────────── */

void setupAPI() {
  server.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("q")) {
      handleCommand(request->getParam("q")->value());
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing parameter ?q=");
    }
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{\"billId\":\"" + billId +
                  "\",\"amount\":" + String(amount, 2) +
                  ",\"status\":\"" + paymentStatus + "\"}";
    request->send(200, "application/json", json);
  });

  server.begin();
}

/* ─────────────────── WIFI ─────────────────── */

void setupWiFi() {
  gfx->fillScreen(BLACK);
  centerText("Connecting WiFi...", 120, 1, WHITE);

  WiFi.begin(ssid, password);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 40) {
    delay(500);
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    gfx->fillScreen(BLACK);
    centerText("WiFi OK", 100, 2, GREEN);
    centerText(WiFi.localIP().toString(), 130, 1, WHITE);
    Serial.println("IP: " + WiFi.localIP().toString());
    delay(1500);
  } else {
    gfx->fillScreen(BLACK);
    centerText("WiFi FAILED", 100, 2, RED);
    centerText("Serial-only mode", 130, 1, WHITE);
    delay(1500);
  }
}

/* ─────────────────── SETUP ─────────────────── */

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  gfx->begin();
  gfx->setRotation(0);
  gfx->fillScreen(BLACK);

  setupWiFi();
  setupAPI();

  screen = SCREEN_WELCOME;
  updateScreen();
}

/* ─────────────────── LOOP ─────────────────── */

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      handleCommand(serialBuffer);
      serialBuffer = "";
    } else if (c != '\r') {
      serialBuffer += c;
    }
  }
}
