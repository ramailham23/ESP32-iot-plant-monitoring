import paho.mqtt.client as mqtt
import mysql.connector
import json

MQTT_BROKER = "broker.emqx.io"
MQTT_PORT = 1883
MQTT_TOPIC = "iot_monitoring/IlhamRama1211/sensor"

DB_CONFIG = {
    "host": "localhost",
    "user": "root",
    "password": "",  # default XAMPP kosong
    "database": "iot_monitoring"
}

def on_connect(client, userdata, flags, rc):
    print("Terhubung ke broker, kode:", rc)
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        print("Data diterima:", payload)

        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor()
        cursor.execute(
            "INSERT INTO sensor_data (device_id, suhu, kelembapan, jarak_air, gas_value) VALUES (%s, %s, %s, %s, %s)",
            (payload["device_id"], payload["suhu"], payload["kelembapan"], payload["jarak_air"], payload["gas_value"])
        )
        conn.commit()
        cursor.close()
        conn.close()
        print("Tersimpan ke MySQL")
    except Exception as e:
        print("Error:", e)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_forever()
