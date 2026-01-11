# CPC357PROJECT_164952_164856
# 🏙️ Urban Noise Monitoring & Classification System

An end-to-end IoT solution utilizing **Edge AI (TinyML)**, **Google Cloud Platform (GCP)**, and **BigQuery** to monitor, classify and visualize urban noise pollution in alignment with **UN Sustainable Development Goal (SDG) 11: Sustainable Cities and Communities**.

## 📌 Project Overview
Urban noise pollution is a significant challenge affecting the health and livability of city residents. This project addresses the issue by deploying an intelligent acoustic sensor using a **Maker Feather S3 (ESP32-S3)** and an **INMP441 Digital Microphone**. Unlike traditional sensors, this system uses an on-device **1D-CNN model** to classify specific sounds (Sirens, Drilling, Car Horns, etc.) locally, ensuring data privacy and reducing cloud latency.

### **Key Features (Code-Specific)**
* **Context-Aware Alerting:** Drilling notifications are prioritized during "Quiet Hours" (8:00 PM - 8:00 AM) to monitor municipal noise violations.
* **Local RMS Noise Gate:** A firmware-level threshold (3150 RMS) filters out silence to optimize power and reduce unnecessary cloud data ingestion.
* **High-Fidelity Sensing:** Utilizes I2S protocol to capture 16kHz audio for robust AI inference.
* **Hybrid Alerting:** Immediate local feedback via status LEDs and remote mobile notifications via Telegram.

---

## 📂 Repository Structure & File Descriptions
To ensure reproducibility, this repository is organized as follows:

* **`cpcproject_inference.ino`**: The main firmware for the Maker Feather S3. It handles I2S audio capture, local TinyML inference, NTP time synchronization and dual communication (MQTT + Telegram).
* **`dataset_prep.py`**: A preprocessing script that resamples the UrbanSound8K dataset to 16kHz, standardizes audio to 4.0s and creates a mixed-audio test set for robust validation.
* **`Background_noise.ipynb`**: A Jupyter notebook used for data engineering. It applies pink noise filters for augmentation and generates a dedicated `background_noise` class to improve model accuracy.
* **`nano_bridge_gcp.py`**: The cloud-side Python bridge deployed on a GCP VM. It subscribes to the MQTT topic and streams incoming detection metadata into Google BigQuery.
* **`requirements.txt`**: List of Python dependencies (librosa, paho-mqtt, etc.) required for the data preparation and cloud bridge scripts.
* **`ei-cpc357-project-arduino-1.0.9.zip`**: The exported Edge Impulse C++ library containing the trained model parameters and inference engine.

---

## 🏗️ System Architecture
The project follows a professional 4-tier IoT architecture:
1.  **Perception Layer:** Maker Feather S3 + INMP441 Microphone (I2S Interface).
2.  **Network Layer:** WiFi connectivity utilizing MQTT (Mosquitto) and HTTPS (Telegram API) protocols.
3.  **Middleware Layer:** Python-based bridge running on a GCP Compute Engine VM.
4.  **Application Layer:** BigQuery warehouse and Looker Studio visualization.



---

## 🛠️ Hardware Setup & Wiring
The **INMP441** was selected for its high-fidelity 24-bit digital output, providing superior acoustic data for AI classification compared to analog alternatives.

### **Pin Mapping & Connectivity**
| Component | Pin | Maker Feather S3 Pin | Breadboard Location |
| :--- | :--- | :--- | :--- |
| **INMP441 Mic** | SD | GPIO 39 | C25 |
| **INMP441 Mic** | VDD | 3V3 | C26 |
| **INMP441 Mic** | GND | GND | C27 |
| **INMP441 Mic** | SCK | GPIO 40 | H25 |
| **INMP441 Mic** | WS | GPIO 38 | H26 |
| **INMP441 Mic** | L/R | GND | H27 |
| **Red LED** | Anode | GPIO A0 | I24 |
| **Green LED** | Anode | GPIO A1 | I30 |



---

## 🚀 Setup & Installation Instructions

### **1. Data Preparation & AI Training**
1.  Install Python dependencies: `pip install -r requirements.txt`.
2.  Execute `dataset_prep.py` to standardize the UrbanSound8K samples.
3.  Run `Background_noise.ipynb` for data augmentation (pink noise) and class generation.
4.  Upload the prepared data to **Edge Impulse Studio**, train the model and export the Arduino library (`.zip`).

### **2. Telegram Alerting Setup**
1.  **Create Bot:** Search for **@BotFather** on Telegram and use `/newbot` to get your **API Token**.
2.  **Create Channel:** Create a **Private Channel** and add your bot as an **Administrator**.
3.  **Retrieve Chat ID:**
    * Search for **@JsonDumpBot** in Telegram.
    * Post a message in your private channel and **Forward** it to **@JsonDumpBot**.
    * Retrieve your **ID** from the `"forward_from_chat"` section (the ID will start with `-100`).

### **3. Google Cloud Platform (GCP) Configuration**
1.  Provision a **Compute Engine VM** and assign a **Static External IP**.
2.  Allow ingress traffic on **TCP Port 1883** in the firewall settings for MQTT communication.
3.  Create a BigQuery dataset and a table named `detection_table`.

### **4. Deployment**
1.  **Backend:** Run the subscriber bridge on your VM: `python3 nano_bridge_gcp.py`.
2.  **Firmware:** Add the Edge Impulse `.zip` library to your Arduino IDE. Update your WiFi, MQTT and Telegram credentials in the code and flash the ESP32-S3.
3.  **Visualization:** Link the BigQuery table to **Looker Studio** for real-time reporting.

---

## 🔗 Project Links
* **YouTube Demo Video:** https://youtu.be/3698FTFiX3U
* **Looker Studio Dashboard:** [CPC357-Project](https://lookerstudio.google.com/) 
* **GitHub Repository:** https://github.com/superyyds/CPC357PROJECT_164952_164856.git

## 👥 Group 30 Details 
* **University:** Universiti Sains Malaysia (USM) 
* **Course:** CPC357 - IoT Architecture & Smart Applications 
* **Deadline:** 11 January 2026, 11pm 
* **Group Members:**
    1. Marcus Tan Tung Chean - 164952 
    2. Ng Zi Jian - 164856 

---
**Disclaimer:** This implementation is original work developed for the CPC357 course and complies with all USM academic integrity guidelines.
