# Local Development Setup

## Running the Mock API Server

This mock server simulates the Arduino REST API endpoints for local testing of the web UI.

### Prerequisites

Install Flask and Flask-CORS (if not already installed):

```bash
pip install flask flask-cors
```

Optional: install live reload support for the mock server:

```bash
pip install livereload
```

### Running the Server

From the project root directory:

```bash
python mock_server.py
```

You should see:

```
======================================================================
ConfigManager Mock API Server
======================================================================
Starting server at http://localhost:5000
Web UI: http://localhost:5000/
API Endpoints:
  - GET http://localhost:5000/api/config  (device config)
  - GET http://localhost:5000/api/ssids   (WiFi networks)
  - GET http://localhost:5000/api/status  (device status)
======================================================================
```

### Accessing the Web UI

Open your browser and navigate to: **http://localhost:5000/**

The page will load and immediately call the mock API endpoints to fetch:
- Device configuration
- Available WiFi networks (SSIDs)
- Device status

### Mock Data

Edit `mock_server.py` to customize the mock responses:

- **MOCK_CONFIG** — Device configuration data
- **MOCK_SSIDS** — WiFi networks list (with signal strength, channel, encryption)
- **MOCK_STATUS** — Device status HTML

### Development Workflow

1. Edit `web_src/index.html` or `web_src/app.js`
2. Save the files
3. Refresh your browser (Flask auto-reloads the files)
4. Changes appear immediately

### Adding More Mock Endpoints

Edit `mock_server.py` and add new routes:

```python
@app.route('/api/settings')
def get_settings():
    """Mock: GET /api/settings"""
    return jsonify({"your": "data"})
```

Then call them from `web_src/app.js`:

```javascript
fetch('/api/settings')
  .then(response => response.json())
  .then(data => console.log(data))
```

### Testing the Separate JS File

Before building for the Arduino, you can test that the JavaScript works correctly:

1. Open browser DevTools (F12)
2. Check the **Console** tab for any errors
3. The separate `app.js` file loads and runs correctly
4. When you build for production, it gets inlined automatically
