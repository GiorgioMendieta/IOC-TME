import paho.mqtt.client as mqtt
from flask import Flask, render_template, request
import json
import sqlite3
from rpi_mqtt_client import mqttc

DATABASE_PATH = "./sensordata.db"

app = Flask(__name__)


def dict_factory(cursor, row):
    d = {}
    for idx, col in enumerate(cursor.description):
        d[col[0]] = row[idx]
    return d


@app.route("/", methods=["GET", " POST"])
def main():
    # connects to SQLite database. File is named "sensordata.db" without the quotes
    # WARNING: your database file should be in the same directory of the app.py file or have the correct path
    conn = sqlite3.connect(DATABASE_PATH)
    conn.row_factory = dict_factory
    c = conn.cursor()
    c.execute("SELECT * FROM prReadings ORDER BY id DESC LIMIT 20")
    readings = c.fetchall()
    # print(readings)

    # POST request because variables will alter the state on the backend
    # if request.method == "POST":

    return render_template("main.html", readings=readings)


# if __name__ == "__main__":
#    app.run(host="0.0.0.0", port=8181, debug=True)
