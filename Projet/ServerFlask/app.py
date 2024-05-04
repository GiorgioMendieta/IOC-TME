from flask import Flask, render_template, request, flash, redirect, url_for
import json
import paho.mqtt.client as mqtt  # for MQTT communication
import sqlite3  # for SQLite database
import datetime  # for timestamp

app = Flask(__name__)

# DATABASE_PATH = "/databases/databaseProjet/ioc_project.db"
DATABASE_PATH = "./ioc_project.db"  # Pathname for testing in local machine
SECRET_KEY = b'_5#y2L"F4Q8z\n\xec]/'

# Ensure templates are auto-reloaded
app.config["TEMPLATES_AUTO_RELOAD"] = True
# Set secret key to flash messages
app.config["SECRET_KEY"] = SECRET_KEY


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
        luminance = message.payload.decode("utf-8")
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
            ),
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

# Comment the next line disable MQTT for testing purposes
if mqttc.connect(MQTT_BROKER, port=1883, keepalive=60) != 0:
    print("Connection failed")
mqttc.loop_start()


def apology(message, code=400):
    """Render message as an apology to user."""

    def escape(s):
        """
        Escape special characters.

        https://github.com/jacebrowning/memegen#special-characters
        """
        for old, new in [
            ("-", "--"),
            (" ", "-"),
            ("_", "__"),
            ("?", "~q"),
            ("%", "~p"),
            ("#", "~h"),
            ("/", "~s"),
            ('"', "''"),
        ]:
            s = s.replace(old, new)
        return s

    return render_template("apology.html", top=code, bottom=escape(message)), code


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

    # LedOn or LedOff
    if request.method == "POST":
        payload = request.form.get("payload")
        # print("Payload: " + payload)
        mqttc.publish("rpi/broadcast", payload)
        flash("Payload sent!")
        # return redirect("/")
        # return redirect(url_for("main"))

    return render_template("index.html", readings=readings)
    # return apology("Not found", 400)


@app.route("/ledOn", methods=["GET", "POST"])
def ledOn():
    if request.method == "POST":
        payload = request.form.get("ledOn")
        # print("Payload: " + payload)
        mqttc.publish("rpi/broadcast", "ledOn")
        flash("Led On!")
        # return redirect("/")
        return redirect("/")

    return render_template("/")


@app.route("/ledOff", methods=["GET", "POST"])
def ledOff():
    if request.method == "POST":
        payload = request.form.get("ledOff")
        # print("Payload: " + payload)
        mqttc.publish("rpi/broadcast", "ledOff")
        flash("Led Off!")
        # return redirect("/")
        return redirect("/")

    return render_template("/")


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8181, debug=True, use_reloader=False)
