#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <driver/i2s.h>
#include "time.h"
#include <CPC357_Project_inferencing.h> 
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// --- CONFIGURATION ---
const char* ssid = "Peeee";
const char* password = "244466666";
const char* mqtt_server = "34.29.181.85"; 
const char* device_id = "makerfeathers301";

// Telegram Bot Configuration
#define BOT_TOKEN "8153754842:AAHWcBv9ng1qQT4R0_dEeL1kQNaycGcQ7kE"
#define CHAT_ID   "-1003408639046"

// I2S Microphone Settings (INMP441)
#define I2S_WS 38
#define I2S_SD 39
#define I2S_SCK 40
#define I2S_PORT I2S_NUM_0

// Model Settings
// Note: This is unused in blocking mode, but we keep it for reference
#define EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW 4 

// PIN DEFINITIONS
#define PIN_LED_GREEN A1 
#define PIN_LED_RED   A0  

// Variables for blinking without delay()
unsigned long previousMillis = 0;
bool ledState = LOW;
String currentAlert = "none";

// Alert cooldown to prevent spam (milliseconds)
unsigned long lastTelegramAlert = 0;
const unsigned long TELEGRAM_COOLDOWN = 60000; 

WiFiClient espClient;
PubSubClient client(espClient);

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// Audio Buffer
int16_t *inference_buffer; 

void setup_external_leds() {
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);
    
    // Test Sequence
    digitalWrite(PIN_LED_GREEN, HIGH); delay(1000);
    digitalWrite(PIN_LED_RED, HIGH);   delay(1000);
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_RED, LOW);
}

void trigger_alert(String label, float confidence) {
    unsigned long currentMillis = millis();

    // 1. DANGER DETECTED (Adjust labels to match your new model!)
    if ((label == "siren" || label == "drilling") && confidence > 0.60) {
        digitalWrite(PIN_LED_GREEN, LOW);
        
        // STROBE RED (Fast 100ms blink)
        if (currentMillis - previousMillis >= 50) {
            previousMillis = currentMillis;
            ledState = !ledState; 
            digitalWrite(PIN_LED_RED, ledState);
        }
    }
    // 2. SAFE / BACKGROUND
    else {
        digitalWrite(PIN_LED_RED, LOW);

        // HEARTBEAT GREEN (Slow 1000ms blink)
        if (currentMillis - previousMillis >= 1000) {
            previousMillis = currentMillis;
            ledState = !ledState;
            digitalWrite(PIN_LED_GREEN, ledState);
        }
    }
}

void sendTelegramAlert(String label, float confidence) {
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastTelegramAlert < TELEGRAM_COOLDOWN) {
        return;
    }

    if (WiFi.status() != WL_CONNECTED) return;
    
    String message = "";
    bool shouldAlert = false;
    
    if (label == "siren" && confidence > 0.70) {
        message = "🚨 *EMERGENCY ALERT*\n\n⚠️ Siren detected nearby!\n";
        shouldAlert = true;
    }
    else if (label == "car_horn" && confidence > 0.75) {
        message = "🚗 *TRAFFIC ALERT*\n\n📢 Loud car horn detected!\n";
        shouldAlert = true;
    }
    else if (label == "drilling" && confidence > 0.70) {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            int hour = timeinfo.tm_hour;
            if (hour < 8 || hour >= 20) {
                message = "🔧 *NOISE VIOLATION*\n\n🚧 Drilling detected during quiet hours!\n";
                shouldAlert = true;
            }
        }
    }
    
    if (shouldAlert) {
        message += "\n📍 Device: " + String(device_id) + "\n";
        message += "📊 Confidence: " + String(confidence * 100, 1) + "%";
        
        if (bot.sendMessage(CHAT_ID, message, "Markdown")) {
            Serial.println("✅ Telegram alert sent!");
            lastTelegramAlert = currentMillis;
        }
    }
}

void setup() {
    Serial.begin(115200);

    // Debug: Print model parameters from the new library
    Serial.println("--- MODEL PARAMETERS ---");
    Serial.print("Project: "); Serial.println("CPC357_Project");
    Serial.print("Window Size (ms): "); Serial.println(EI_CLASSIFIER_INTERVAL_MS * EI_CLASSIFIER_RAW_SAMPLE_COUNT);
    Serial.print("Sample Count: "); Serial.println(EI_CLASSIFIER_RAW_SAMPLE_COUNT); // Should be 32000 for 2000ms
    Serial.print("Frequency: "); Serial.println(EI_CLASSIFIER_FREQUENCY); // Should be 16000
    Serial.println("------------------------");

    setup_external_leds();
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected");

    // GMT+8
    const long gmtOffset_sec = 8 * 3600;
    const int daylightOffset_sec = 0;
    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");

    client.setServer(mqtt_server, 1883);
    secured_client.setInsecure();

    // Allocate buffer based on the NEW model size automatically
    inference_buffer = (int16_t *)malloc(EI_CLASSIFIER_RAW_SAMPLE_COUNT * sizeof(int16_t));
    if (!inference_buffer) {
        Serial.println("ERR: Failed to allocate audio buffer!");
        while(1);
    }

    // I2S Initialization
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000, // Matches your new model frequency
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = -1,
        .data_in_num = I2S_SD
    };
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
}

void capture_audio() {
    size_t bytesIn = 0;
    
    // This will now block for 2000ms (based on EI_CLASSIFIER_RAW_SAMPLE_COUNT)
    esp_err_t result = i2s_read(I2S_PORT, (char *)inference_buffer, EI_CLASSIFIER_RAW_SAMPLE_COUNT * sizeof(int16_t), &bytesIn, portMAX_DELAY);
    
    if (result == ESP_OK) {
        Serial.print("Audio captured. Bytes: ");
        Serial.println(bytesIn);
    } else {
        Serial.print("ERR: I2S Read Failed! Error code: ");
        Serial.println(result);
    }
}

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    for (size_t i = 0; i < length; i++) {
        out_ptr[i] = (float)inference_buffer[offset + i];
    }
    return 0;
}

void preprocess_audio(int16_t *buffer, size_t size) {
    long sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += buffer[i];
    }
    int16_t dc_offset = sum / size;
    for (size_t i = 0; i < size; i++) {
        buffer[i] -= dc_offset;
    }
}

bool is_loud_enough_rms(int16_t *buffer, size_t size, float threshold) {
    double sum_squares = 0;
    for (size_t i = 0; i < size; i++) {
        sum_squares += (double)buffer[i] * buffer[i];
    }
    float rms = sqrt(sum_squares / size);
    
    Serial.print("RMS Volume: ");
    Serial.println(rms);
    
    return (rms > threshold);
}

void loop() {
    // 1. MQTT Connection
    if (!client.connected()) {
        if (client.connect(device_id)) {
            Serial.println("MQTT Connected");
        }
    }
    client.loop();

    // 2. Capture (Takes 2.0 seconds now)
    Serial.println("Listening (2.0s)...");
    capture_audio();
    preprocess_audio(inference_buffer, EI_CLASSIFIER_RAW_SAMPLE_COUNT);

    // --- NOISE GATE ---
    // You might need to adjust this 1600 threshold slightly 
    // because RMS can vary with window size, though usually it's stable.
    if (!is_loud_enough_rms(inference_buffer, EI_CLASSIFIER_RAW_SAMPLE_COUNT, 3150)) {
        Serial.println("Silence. Skipping.");
        trigger_alert("background_noise", 0.0);
        return; 
    }

    // 3. Inference
    Serial.println("Inferencing...");
    signal_t signal;
    signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
    signal.get_data = &raw_feature_get_data;
    ei_impulse_result_t result;

    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

    if (res == EI_IMPULSE_OK) {
        
        StaticJsonDocument<512> doc;
        doc["device_id"] = device_id;
        
        struct tm timeinfo;
        if(getLocalTime(&timeinfo)){
            char timeStringBuff[50];
            strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S %Z", &timeinfo);
            doc["timestamp"] = timeStringBuff;
        } else {
            doc["timestamp"] = "1970-01-01 00:00:00"; 
        }

        JsonArray labels = doc.createNestedArray("labels");
        JsonArray confidence = doc.createNestedArray("confidence");

        bool noise_detected = false;
        String detected_label = "background_noise";
        float detected_confidence = 0.0;

        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            // Print results for debugging
            Serial.print(result.classification[ix].label);
            Serial.print(": ");
            Serial.print(result.classification[ix].value, 2);
            Serial.print(" | ");

            String label = result.classification[ix].label;
            float value = result.classification[ix].value;

            // Find highest confidence
            if (value > detected_confidence) {
                detected_confidence = value;
                detected_label = label;
            }
            
            // Only add to MQTT if confidence is high (> 0.7)
            if (value > 0.7 && label != "background_noise") { 
                 labels.add(label);
                 confidence.add(value);
                 noise_detected = true;
            }
        }
        Serial.println();

        // Trigger Alerts
        trigger_alert(detected_label, detected_confidence);
        sendTelegramAlert(detected_label, detected_confidence);
        
        // 4. Publish to MQTT if noise found
        if (noise_detected) {
            char jsonBuffer[512];
            serializeJson(doc, jsonBuffer);
            client.publish("urban/noise", jsonBuffer);
            Serial.println("🚀 Published to MQTT!");
        }
    } else {
        Serial.print("ERR: Classifier failed: ");
        Serial.println(res);
    }
}