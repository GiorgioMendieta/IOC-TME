import os

from flask import Flask

# Configure application
app = Flask(__name__)

# Run app in debug mode
app.config["DEBUG"] = True
# SQLAlchemy config
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///db.sqlite3'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

# IMPORTANT: Import must be after declaring app to avoid a circular import
from database import db, Messages