# MQTT Sensor Network: Broker, Publisher, and Subscriber

This project implements a custom MQTT-like communication protocol with support for multiple topics and wildcard filtering.

## Components

- **Broker (C)**: Accepts multiple connections and routes messages from publishers to subscribers based on topics.
- **Subscriber (C)**: Connects to broker and listens for messages matching its topic (supports `+` wildcard).
- **Publisher (ESP32 + FreeRTOS)**: Sends real-time temperature and humidity data over Wi-Fi using sockets.

## Features

- Thread-based concurrency in the broker
- Forked process-based subscribers
- Mutex and semaphore usage for IPC
- Logging system to track message flow
- Wildcard `+` support in topic filtering
- Real sensor simulation via Wokwi

## Start the broker

./broker <IP_ADDRESS> <PORT>

## Start the subscriber

./subscriber <IP_ADDRESS> <PORT> <TOPIC>


