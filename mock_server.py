#!/usr/bin/env python3
"""
Mock API Server for ConfigManager Web UI Development
Simulates Arduino REST API endpoints for local testing
"""

import json
import os
import random
import time
from pathlib import Path

try:
    from flask import Flask, jsonify, request, send_from_directory
    from flask_cors import CORS
    FLASK_AVAILABLE = True
except ImportError:
    FLASK_AVAILABLE = False

try:
    from livereload import Server
    LIVERELOAD_AVAILABLE = True
except ImportError:
    LIVERELOAD_AVAILABLE = False

app = Flask(__name__, static_folder='web_src', static_url_path='')

# Enable CORS for local testing
CORS(app)

# ============================================================================
# Mock Data
# ============================================================================

MOCK_ABOUT = {
    "page_title": "Scheid Product Configuration",
    "device_name": "ESP8266 Sensor",
    "version": "1.0.1"
}

MOCK_SSIDS = {
    "ssids": [
        {
            "ssid": "MyHomeNetwork",
            "rssi": -45,
            "channel": 6,
            "encryption": "WPA2"
        },
        {
            "ssid": "GuestWiFi",
            "rssi": -62,
            "channel": 11,
            "encryption": "WPA2"
        },
        {
            "ssid": "NeighborNetwork",
            "rssi": -72,
            "channel": 1,
            "encryption": "WPA"
        },
        {
            "ssid": "OpenNetwork",
            "rssi": -58,
            "channel": 6,
            "encryption": "Open"
        }
    ]
}

def jitter_ssid_signals():
    """Randomly jitter RSSI values for a more realistic WiFi rescan."""
    for ssid in MOCK_SSIDS['ssids']:
        # Random frequency: sometimes many changes, sometimes few
        if random.random() < random.uniform(0.25, 0.75):
            change = random.choice([-2, -1, 1, 2])
            ssid['rssi'] = max(-90, min(-30, ssid['rssi'] + change))

MOCK_STATUS = [
  ["Temperature", "25.4°C"],
  ["Humidity", "60%"],
  ["WiFi Signal", "-45 dBm (Good)"],
  ["Uptime", "12 hours 34 minutes"],
  ["Last Sync", "2 minutes ago"]
]

# ============================================================================
# Routes
# ============================================================================

@app.route('/')
def serve_index():
    """Serve the main index.html"""
    return send_from_directory('web_src', 'index.html')

@app.route('/<path:filename>')
def serve_static(filename):
    """Serve static files (CSS, JS, etc.)"""
    return send_from_directory('web_src', filename)

@app.route('/api/about')
def get_about():
    """Mock: GET /api/about - Returns info about device and firmwares"""
    return jsonify(MOCK_ABOUT)

@app.route('/api/ssids')
def get_ssids():
    """Mock: GET /api/ssids - Returns available WiFi networks"""
    # Simulate scanning for WiFi networks (2 second delay)
    time.sleep(2)
    jitter_ssid_signals()
    return jsonify(MOCK_SSIDS)

@app.route('/api/connect', methods=['POST'])
def connect_ssid():
    """Mock: POST /api/connect - Simulates a WiFi connection attempt."""
    if not request.is_json:
        return jsonify({"success": False, "error": "Request payload must be JSON."}), 400

    payload = request.get_json()
    ssid = payload.get('ssid')
    password = payload.get('password', '')

    if not ssid:
        return jsonify({"success": False, "error": "SSID is required."}), 400

    network = next((entry for entry in MOCK_SSIDS['ssids'] if entry['ssid'] == ssid), None)
    if network is None:
        return jsonify({"success": False, "error": "SSID not found."}), 404

    if network['encryption'] != 'Open':
        if len(password) < 8:
            return jsonify({"success": False, "error": "Password must be at least 8 characters."}), 400
        if random.random() < 0.2:
            return jsonify({"success": False, "error": "Authentication failed. Please check your password."}), 403

    # Simulate connection time
    time.sleep(1)
    return jsonify({"success": True, "message": f"Connected to {ssid}."}), 200

@app.route('/api/status')
def get_status():
    """Mock: GET /api/status - Returns device status as HTML"""
    return MOCK_STATUS, 200, {'Content-Type': 'text/html'}

# ============================================================================
# Main
# ============================================================================

if __name__ == '__main__':
    if not FLASK_AVAILABLE:
        print("ERROR: Flask is not installed")
        print("Install with: pip install flask flask-cors")
        exit(1)
    
    print("=" * 70)
    print("ConfigManager Mock API Server")
    print("=" * 70)
    print(f"Starting server at http://localhost:5000")
    print(f"Web UI: http://localhost:5000/")
    print(f"API Endpoints:")
    print(f"  - GET http://localhost:5000/api/about  (device config)")
    print(f"  - GET http://localhost:5000/api/ssids   (WiFi networks)")
    print(f"  - GET http://localhost:5000/api/status  (device status)")
    print("=" * 70)
    print()

    if LIVERELOAD_AVAILABLE:
        print("Live reload enabled for web_src files")
        server = Server(app.wsgi_app)
        server.watch('web_src/*.html')
        server.watch('web_src/**/*.html')
        server.watch('web_src/**/*.js')
        server.watch('web_src/**/*.css')
        server.watch('web_src/**/*.json')
        server.serve(host='localhost', port=5000, debug=True)
    else:
        app.run(debug=True, host='localhost', port=5000)
