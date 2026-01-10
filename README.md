# CPC357PROJECT_164952_164856
# 🏙️ Urban Noise Monitoring & Classification System

An end-to-end IoT solution utilizing **Edge AI (TinyML)**, **Google Cloud Platform (GCP)**, and **BigQuery** to monitor, classify, and visualize urban noise pollution in alignment with **UN Sustainable Development Goal (SDG) 11: Sustainable Cities and Communities**.

## 📌 Project Overview

Urban noise pollution is a significant challenge affecting the health and livability of city residents. This project addresses the issue by deploying an intelligent acoustic sensor using a **Maker Feather S3 (ESP32-S3)** and an **INMP441 Digital Microphone**3. Unlike traditional sensors, this system uses an on-device **1D-CNN model** to classify specific sounds (Sirens, Drilling, Car Horns, etc.) locally, ensuring data privacy and reducing cloud latency.

### **Key Features**

- **Edge AI Inference:** Real-time multi-label sound classification performed on-device.
- **RMS Noise Gate:** Energy-based filtering in firmware to optimize power and reduce unnecessary cloud ingestion.
- **Direct-to-App Alerting:** Priority SSL/TLS alerts sent directly from the hardware to **Telegram** for emergency events (Sirens/Drilling).
- **Cloud Data Pipeline:** Secure MQTT streaming to **Google BigQuery** for long-term historical storage.
- **Live Analytics:** Interactive **Looker Studio** dashboard for urban planning and trend analysis.

---

## 🏗️ System Architecture

The project follows a 4-tier IoT architecture:

1. **Perception Layer:** Maker Feather S3 + INMP441 Microphone (I2S Interface).
2. **Network Layer:** WiFi connectivity utilizing MQTT (Mosquitto) and HTTPS (Telegram API).
3. **Middleware Layer:** Python-based bridge running on a GCP Compute Engine VM.
4. **Application Layer:** BigQuery warehouse and Looker Studio visualization.

---

## 🛠️ Hardware Setup & Wiring

The choice of the **INMP441** is justified by its high-fidelity digital output, which provides superior acoustic data for AI classification compared to analog alternatives.

### **Wiring & Pin Mapping**

| **Component** | **Pin** | **Maker Feather S3 Pin** | **Breadboard Location** |
| --- | --- | --- | --- |
| **INMP441 Mic** | SD | GPIO 39 | C25 |
| **INMP441 Mic** | VDD | 3V3 | C26 |
| **INMP441 Mic** | GND | GND | C27 |
| **INMP441 Mic** | SCK | GPIO 40 | H25 |
| **INMP441 Mic** | WS | GPIO 38 | H26 |
| **INMP441 Mic** | L/R | GND | H27 |
| **Red LED** | Anode | GPIO A0 | I24 |
| **Green LED** | Anode | GPIO A1 | I30 |

---

## 💻 Software Dependencies & Requirements

The following libraries and environments must be installed to reproduce the system:

### **1. Arduino IDE (Edge Tier)**

- **Edge Impulse SDK:** Your custom exported library from Edge Impulse Studio.
- **Paho MQTT:** To manage communication with the GCP broker.
- **ArduinoJson:** To serialize classification results into JSON strings.
- **UniversalTelegramBot:** To interface with the Telegram API.

### **2. Cloud Tier (GCP VM)**

- **Mosquitto Broker:** MQTT middleware installed on a Linux VM.
- **Python 3.8+:** With the following packages:Bash
    
    `pip install paho-mqtt google-cloud-bigquery`
    

---

🚀 Setup & Installation Instructions 

### **1. AI Model Training**

- Upload audio datasets to **Edge Impulse Studio**.
- Train a **Conv1D** classification model using **Sigmoid** activation and **Binary Crossentropy**.
- Export the model as an **Arduino Library** and add it to your IDE.

### **2. Telegram Alerting Setup**

- Create a bot via **@BotFather** to obtain an **API Token**.
- Create a private channel and add the bot as an **Administrator**.
- Retrieve the **Chat ID** (typically starts with `100`).

### **3. Cloud Infrastructure (GCP)**

- Provision a **Compute Engine VM** and allow traffic on **TCP Port 1883**.
- Create a BigQuery dataset and a table named `detection_table`.

### **4. Deployment**

- **Backend:** Run the Python bridge on your VM: `python3 cloud_bridge.py`.
- **Firmware:** Update the Arduino sketch with WiFi, MQTT, and Telegram credentials; then flash the ESP32-S3.
- **Visualization:** Connect BigQuery to **Looker Studio** for real-time reporting.

---

🔗 Project Links

- **YouTube Demo Video:** [INSERT YOUR VIDEO LINK HERE]
- **Looker Studio Dashboard:** [CPC357-Project](https://lookerstudio.google.com/u/0/reporting/f1bddb94-dd8f-44fd-a2ad-c9480ff11f2c/page/KJSkF/edit)
- **GitHub Repository:** [INSERT YOUR GITHUB LINK HERE]

👥 Group 30 Details 

- **Course:** CPC357 - IoT Architecture & Smart Applications
- **University:** Universiti Sains Malaysia (USM)
- **Deadline:** 11 January 2026
- **Group Members:**
    1. Marcus Tan Tung Chean - 164952
    2. Ng Zi jian - 164856
