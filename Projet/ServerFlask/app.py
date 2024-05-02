import paho.mqtt.client as mqtt
from flask import Flask, render_template, request
import json
import sqlite3
import datetime  # for timestamp

DATABASE_PATH = "/databases/databaseProjet/ioc_project.db"

app = Flask(__name__)


def dict_factory(cursor, row):
    d = {}
    for idx, col in enumerate(cursor.description):
        d[col[0]] = row[idx]
    return d


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc, props=None):
    print("Connected with result code " + str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("esp32/photoVal")


# The callback for when a PUBLISH message is received from the ESP32
def on_message(client, userdata, message):
    if message.topic == "esp32/photoVal":

        # esp32readings_json = json.loads(message.payload)
        luminance = message.payload.decode('utf-8')
        print("ESP32 luminance: " + luminance)

        # connects to SQLite database. File is named "ioc_project.db" without the quotes
        # WARNING: your database file should be in the same directory of the app.py file or have the correct path
        conn = sqlite3.connect(
            DATABASE_PATH, detect_types=sqlite3.PARSE_DECLTYPES | sqlite3.PARSE_COLNAMES
        )
        c = conn.cursor()
        currentDateTime = datetime.datetime.now(
            datetime.UTC
        )  # current date and time at the time of message

        insertQuery = """INSERT INTO IOTSensors 
            (deviceName, sensor, reading, timestamp) 
            VALUES((?), (?), (?), (?))"""  # Add ; at the end?

        c.execute(
            insertQuery,
            (
                "ESP32",
                "photoresistance",
                luminance,
                currentDateTime,
            )
        )
        print("ESP32 readings updated")

        # commit the changes,
        # close the cursor and database connection
        conn.commit()
        c.close()
        conn.close()


MQTT_BROKER = "192.168.1.3"

mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqttc.on_connect = on_connect
mqttc.on_message = on_message

if mqttc.connect(MQTT_BROKER, port=1883, keepalive=60) != 0:
    print("Connection failed")
mqttc.loop_start()


@app.route("/", methods=["GET", " POST"])
def main():
    # connects to SQLite database. File is named "sensordata.db" without the quotes
    # WARNING: your database file should be in the same directory of the app.py file or have the correct path
    conn = sqlite3.connect(DATABASE_PATH)
    conn.row_factory = dict_factory
    c = conn.cursor()
    c.execute("SELECT * FROM IOTSensors ORDER BY id DESC LIMIT 20")
    readings = c.fetchall()
    # print(readings)

    # POST request because variables will alter the state on the backend
    # if request.method == "POST":

    return render_template("main.html", readings=readings)


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8181, debug=True)
