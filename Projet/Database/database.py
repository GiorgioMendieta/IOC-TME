# Import app from app.py
from app import app
from flask_sqlalchemy import SQLAlchemy

db = SQLAlchemy(app)

# Table structure of database (schema)

# id, Topic, Payload, Timestamp
class Messages(db.Model):
    __tablename__ = 'messages'

    id = db.Column(db.Integer, primary_key=True)
    clientMqtt = db.Column(db.String(100), unique=True, nullable=False)
    topic = db.Column(db.String(100), unique=False, nullable=False)
    payload = db.Column(db.String(100), unique=False, nullable=False)
    timestamp = db.Column(db.String(100), unique=False, nullable=False)
    # One-to-one relationship
    # stats = db.relationship("Stats", backref="messages")

    # Representation method
    def __repr__(self):
        return f'<{self.clientMqtt} message: {self.payload}>'