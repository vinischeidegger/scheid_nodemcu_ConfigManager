let currentConfig = null;
let ssidRefreshTimer = null;
let wifiConnectionInProgress = false;

const NETWORK_DEFAULTS = Object.freeze({
  // ESP8266/ESP32 SoftAP default gateway; override via window.__CM_API_AP_ORIGIN__ when needed.
  apOrigin: 'http://192.168.4.1'
});

function normalizeOrigin(origin) {
  return String(origin || '').trim().replace(/\/$/, '');
}

function getApiBootstrapConfig() {
  const query = new URLSearchParams(window.location.search || '');
  const queryOrigin = query.get('api_origin');
  const windowOrigin = window.location?.origin;
  const overrideOrigin = window.__CM_API_ORIGIN__;
  const apOriginOverride = window.__CM_API_AP_ORIGIN__;

  return {
    preferredOrigin: normalizeOrigin(queryOrigin || overrideOrigin || windowOrigin),
    apFallbackOrigin: normalizeOrigin(apOriginOverride || NETWORK_DEFAULTS.apOrigin)
  };
}

function getApiBaseCandidates() {
  const { preferredOrigin, apFallbackOrigin } = getApiBootstrapConfig();
  return [...new Set([preferredOrigin, apFallbackOrigin].filter(Boolean))];
}

async function apiFetch(path, options = {}) {
  const normalizedPath = path.startsWith('/') ? path : `/${path}`;
  const bases = getApiBaseCandidates();
  let lastError = null;

  for (const base of bases) {
    try {
      return await fetch(`${base}${normalizedPath}`, options);
    } catch (err) {
      lastError = err;
    }
  }

  throw lastError || new Error('Failed to reach API');
}

function getParameterValueMap(configData = currentConfig) {
  const map = {};
  const parameters = configData?.parameters;
  if (!Array.isArray(parameters)) {
    return map;
  }

  parameters.forEach(param => {
    if (param && typeof param.id === 'string') {
      map[param.id] = param.value;
    }
  });
  return map;
}

function showMqttMessage(message, type = 'info') {
  const messageElement = document.getElementById('mqtt-message');
  if (!messageElement) {
    return;
  }

  messageElement.textContent = message;
  messageElement.className = `wifi-message ${type}`;
  messageElement.classList.remove('hidden');
}

function populateMqttForm(configData = currentConfig) {
  const params = getParameterValueMap(configData);
  const mqttEnabled = document.getElementById('mqttEnabled');
  const mqttServer = document.getElementById('mqttServer');
  const mqttPort = document.getElementById('mqttPort');
  const mqttUser = document.getElementById('mqttUser');
  const mqttPassword = document.getElementById('mqttPassword');
  const mqttTopic = document.getElementById('mqttTopic');
  const mqttPublishInterval = document.getElementById('mqttPublishInterval');

  if (mqttEnabled) mqttEnabled.checked = Boolean(params.mqtt_enabled);
  if (mqttServer && typeof params.mqtt_server === 'string') mqttServer.value = params.mqtt_server;
  if (mqttPort && params.mqtt_port !== undefined) mqttPort.value = params.mqtt_port;
  if (mqttUser && typeof params.mqtt_username === 'string') mqttUser.value = params.mqtt_username;
  if (mqttPassword && typeof params.mqtt_password === 'string') mqttPassword.value = params.mqtt_password;
  if (mqttTopic && typeof params.mqtt_base_topic === 'string') mqttTopic.value = params.mqtt_base_topic;
  if (mqttPublishInterval && params.mqtt_publish_interval_ms !== undefined) mqttPublishInterval.value = params.mqtt_publish_interval_ms;
}

/**
 * Load device configuration from the mock API and update the page title.
 */
async function loadConfig() {
  try {
    const response = await apiFetch('/api/config');
    if (!response.ok) {
      throw new Error('Failed to load config');
    }
    const data = await response.json();
    currentConfig = data;
    const titleElement = document.getElementById('pageTitle');
    if (titleElement) {
      titleElement.textContent = data.page_title || titleElement.textContent;
    }
    document.title = data.page_title || document.title;
    updateWifiSetupDetails(data);
    populateMqttForm(data);
    console.log('ConfigManager state loaded:', data);
    return data;
  } catch (error) {
    console.error('Error loading config:', error);
    return null;
  }
}

function getWifiModeLabel(configData = currentConfig) {
  const wifiModeValue = configData?.wifi_mode || configData?.apMode || configData?.ap_mode;
  if (!wifiModeValue) {
    return 'Unknown';
  }

  return String(wifiModeValue).toUpperCase();
}

function updateWifiSetupDetails(configData = currentConfig) {
  const modeElement = document.getElementById('wifi-mode-value');
  const apSsidElement = document.getElementById('wifi-ap-ssid-value');
  const staSsidElement = document.getElementById('wifi-sta-ssid-value');

  if (!modeElement || !apSsidElement || !staSsidElement) {
    return;
  }

  modeElement.textContent = getWifiModeLabel(configData);
  apSsidElement.textContent = configData?.ap_ssid || 'Not available';
  staSsidElement.textContent = configData?.wifi_ssid || 'Not configured';
}

/**
 * Returns true when the WiFi page is currently active based on the URL hash.
 * @returns {boolean}
 */
function isWifiSectionActive() {
  return window.location.hash === '#wifi';
}

/**
 * Classify RSSI into a WiFi strength level and label.
 * @param {number} rssi - Received signal strength in dBm.
 * @returns {{level: number, label: string}}
 */
function getWifiStrength(rssi) {
  if (rssi >= -40) {
    return { level: 5, label: 'Excellent' };
  }
  if (rssi >= -55) {
    return { level: 4, label: 'Good' };
  }
  if (rssi >= -65) {
    return { level: 3, label: 'Fair' };
  }
  if (rssi >= -75) {
    return { level: 2, label: 'Weak' };
  }
  if (rssi >= -85) {
    return { level: 1, label: 'Poor' };
  }
  return { level: 0, label: 'No Signal' };
}

/**
 * Render a set of signal bars for the specified strength level.
 * @param {number} level - Active bar count from 0 to 5.
 * @returns {string} HTML markup for the bars.
 */
function renderSignalBars(level) {
  let bars = '';
  for (let i = 1; i <= 5; i++) {
    bars += `<div class="signal-bar${i <= level ? ' active' : ''}" style="height: ${4 + i * 3}px"></div>`;
  }
  return bars;
}

/**
 * Show a status message below the SSID list.
 * @param {string} message
 * @param {'info'|'error'} type
 */
function showWifiMessage(message, type = 'info') {
  const messageElement = document.getElementById('wifi-message');
  if (!messageElement) {
    return;
  }

  messageElement.textContent = message;
  messageElement.className = `wifi-message ${type}`;
  messageElement.classList.remove('hidden');
}

function clearWifiMessage() {
  const messageElement = document.getElementById('wifi-message');
  if (!messageElement) {
    return;
  }

  messageElement.textContent = '';
  messageElement.className = 'wifi-message hidden';
}

function getPasswordToggleIcon(visible) {
  if (visible) {
    return `
      <svg viewBox="0 0 24 24" aria-hidden="true" focusable="false">
        <path d="M3 3l18 18"></path>
        <path d="M10.6 10.6a3 3 0 0 0 4.24 4.24"></path>
        <path d="M9.9 5.1A10.8 10.8 0 0 1 12 5c6.5 0 10 7 10 7a20.8 20.8 0 0 1-4 4.8"></path>
        <path d="M6.2 6.3C3.6 8 2 12 2 12s3.5 7 10 7c1.7 0 3.2-.5 4.5-1.2"></path>
      </svg>
    `;
  }

  return `
    <svg viewBox="0 0 24 24" aria-hidden="true" focusable="false">
      <path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7-10-7-10-7z"></path>
      <circle cx="12" cy="12" r="3"></circle>
    </svg>
  `;
}

function setPasswordVisibility(visible) {
  const passwordInput = document.getElementById('ssid-password');
  const toggleButton = document.getElementById('togglePasswordVisibility');
  if (!passwordInput || !toggleButton) {
    return;
  }

  passwordInput.type = visible ? 'text' : 'password';
  toggleButton.innerHTML = getPasswordToggleIcon(visible);
  toggleButton.setAttribute('aria-pressed', visible ? 'true' : 'false');
  toggleButton.setAttribute('aria-label', visible ? 'Hide password' : 'Show password');
}

function openPasswordDialog(ssid) {
  const dialog = document.getElementById('password-dialog');
  const ssidLabel = document.getElementById('password-ssid');
  const passwordInput = document.getElementById('ssid-password');
  const errorElement = document.getElementById('password-error');

  if (!dialog || !ssidLabel || !passwordInput || !errorElement) {
    return;
  }

  ssidLabel.textContent = ssid;
  passwordInput.value = '';
  setPasswordVisibility(false);
  errorElement.textContent = '';
  errorElement.classList.add('hidden');
  dialog.classList.remove('hidden');
  passwordInput.focus();
}

function closePasswordDialog() {
  const dialog = document.getElementById('password-dialog');
  setPasswordVisibility(false);
  if (!dialog) {
    return;
  }

  dialog.classList.add('hidden');
}

function setPasswordDialogError(message) {
  const errorElement = document.getElementById('password-error');
  if (!errorElement) {
    return;
  }

  errorElement.textContent = message;
  errorElement.classList.remove('hidden');
}

async function attemptConnect(ssid, password = '') {
  wifiConnectionInProgress = true;
  stopWifiRefresh();
  clearWifiMessage();
  showWifiMessage(`Connecting to ${ssid}... The device may restart and the WiFi list will stop updating.`, 'info');

  try {
    const response = await apiFetch('/api/connect', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded; charset=UTF-8'
      },
      body: new URLSearchParams({ ssid, password })
    });

    const result = await response.json();
    if (!response.ok) {
      throw new Error(result.error || 'Failed to connect');
    }

    showWifiMessage(result.message || `Connected to ${ssid}. Restarting...`, 'info');
  } catch (error) {
    wifiConnectionInProgress = false;
    if (isWifiSectionActive()) {
      startWifiRefresh();
    }
    showWifiMessage(error.message || 'Connection failed.', 'error');
  }
}

async function returnToApMode() {
  const shouldReturnToAp = window.confirm(
    'This will clear saved WiFi credentials and restart the device in AP mode. Continue?'
  );

  if (!shouldReturnToAp) {
    return;
  }

  wifiConnectionInProgress = true;
  stopWifiRefresh();
  clearWifiMessage();
  showWifiMessage('Switching to AP mode... The device will restart shortly.', 'info');

  try {
    const response = await apiFetch('/api/return-to-ap', {
      method: 'POST'
    });
    const result = await response.json();

    if (!response.ok) {
      throw new Error(result.error || 'Failed to switch to AP mode.');
    }

    showWifiMessage(result.message || 'Returning to AP mode. Device restarting...', 'info');
  } catch (error) {
    wifiConnectionInProgress = false;
    if (isWifiSectionActive()) {
      startWifiRefresh();
    }
    showWifiMessage(error.message || 'Failed to switch to AP mode.', 'error');
  }
}

function getSelectedSsidRow() {
  return document.querySelector('.ssid-row.selected');
}

function getSelectedNetwork() {
  const row = getSelectedSsidRow();
  if (!row) {
    return null;
  }

  return {
    ssid: row.dataset.ssidValue,
    secure: row.dataset.security !== 'Open'
  };
}

function handleConnectAction() {
  const network = getSelectedNetwork();
  if (!network) {
    showWifiMessage('Please select a WiFi network before connecting.', 'error');
    return;
  }

  if (network.secure) {
    openPasswordDialog(network.ssid);
    return;
  }

  attemptConnect(network.ssid);
}

/**
 * Fetch the latest available SSIDs and render them into the WiFi selection list.
 * Preserves the currently selected SSID if it is still present after refresh.
 */
async function loadWifiSsids() {
  if (wifiConnectionInProgress) {
    return;
  }

  try {
    const listDiv = document.getElementById('ssid-list');
    if (!listDiv) {
      return;
    }

    const currentSelection = document.querySelector('.ssid-row.selected')?.dataset.ssidValue || document.querySelector('input[name="ssid"]:checked')?.value || null;

    // Show loading indicator in fixed container
    const loadingContainer = document.getElementById('ssid-loading-container');
    if (loadingContainer) {
      loadingContainer.innerHTML = '<div class="spinner" style="margin-left: auto;"></div>';
    }

    const response = await apiFetch('/api/ssids');
    const data = await response.json();
    let ssids = data.ssids;

    // Sort by signal strength (RSSI) - higher (less negative) values first
    ssids.sort((a, b) => b.rssi - a.rssi);

    // Clear the list
    listDiv.innerHTML = '';

    // Create table header
    const headerRow = document.createElement('div');
    headerRow.className = 'ssid-row ssid-header';
    headerRow.innerHTML = `
      <div class="ssid-name-header">Network Name</div>
      <div>Signal Quality</div>
      <div>Security</div>
    `;
    listDiv.appendChild(headerRow);

    // Create data rows
    ssids.forEach((info, index) => {
      const row = document.createElement('div');
      row.className = 'ssid-row';
      row.dataset.ssidValue = info.ssid;
      row.dataset.security = info.encryption;
      const strength = getWifiStrength(info.rssi);
      row.innerHTML = `
        <input id="ssid-${index}" type="radio" name="ssid" value="${info.ssid}">
        <span class="ssid-name">${info.ssid}</span>
        <div class="ssid-signal-cell">
          <div class="signal-bars">${renderSignalBars(strength.level)}</div>
          <span class="signal-label">${strength.label}</span>
        </div>
        <span>${info.encryption !== 'Open' ? '🔒 ' + info.encryption : 'Open'}</span>
      `;

      const radio = row.querySelector('input[type="radio"]');
      row.addEventListener('click', () => {
        radio.checked = true;
        document.querySelectorAll('.ssid-row').forEach(r => r.classList.remove('selected'));
        row.classList.add('selected');
      });

      // keep selection state if radio is clicked directly
      radio.addEventListener('change', () => {
        document.querySelectorAll('.ssid-row').forEach(r => r.classList.remove('selected'));
        if (radio.checked) {
          row.classList.add('selected');
        }
      });

      listDiv.appendChild(row);
    });

    // Restore selection if the previously selected SSID still exists
    if (currentSelection) {
      const selectedRow = Array.from(document.querySelectorAll('.ssid-row')).find(r => r.dataset.ssidValue === currentSelection);
      if (selectedRow) {
        const selectedRadio = selectedRow.querySelector('input[type="radio"]');
        if (selectedRadio) {
          selectedRadio.checked = true;
        }
        document.querySelectorAll('.ssid-row').forEach(r => r.classList.remove('selected'));
        selectedRow.classList.add('selected');
      }
    }

    // Hide loading indicator
    if (loadingContainer) {
      loadingContainer.innerHTML = '';
    }
  } catch (err) {
    if (wifiConnectionInProgress) {
      return;
    }

    console.error('Failed to fetch SSIDs', err);
    const listDiv = document.getElementById('ssid-list');
    if (listDiv) {
      listDiv.innerHTML += '<p style="color: red;">Error loading WiFi networks</p>';
    }
    const loadingContainer = document.getElementById('ssid-loading-container');
    if (loadingContainer) {
      loadingContainer.innerHTML = '';
    }
  }
}

/**
 * Stop periodic WiFi list refresh when the WiFi page is hidden.
 */
function stopWifiRefresh() {
  if (ssidRefreshTimer !== null) {
    clearInterval(ssidRefreshTimer);
    ssidRefreshTimer = null;
  }
}

/**
 * Start periodic refresh of the WiFi SSID list when the WiFi page is active.
 */
function startWifiRefresh() {
  if (ssidRefreshTimer !== null) {
    return;
  }

  if (wifiConnectionInProgress) {
    return;
  }

  if (!isWifiSectionActive()) {
    return;
  }

  loadWifiSsids();
  ssidRefreshTimer = setInterval(() => {
    if (isWifiSectionActive()) {
      loadWifiSsids();
    }
  }, 5000);
}

/**
 * Enable or disable WiFi refresh based on the current visible page.
 */
function updateWifiRefreshState() {
  if (isWifiSectionActive()) {
    startWifiRefresh();
  } else {
    stopWifiRefresh();
  }
}

/**
 * Fetch status HTML from the mock API and render it into the main content area.
 */
async function fetchData() {
  const mainContent = document.getElementById('mainContent');
  if (!mainContent) {
    return;
  }

  const config = await loadConfig();
  const wifiMode = getWifiModeLabel(config);

  try {
    const response = await apiFetch('/api/status');
    if (!response.ok) {
      const errorBody = await response.text();
      throw new Error(`Status request failed (${response.status}): ${errorBody}`);
    }

    const data = await response.json();
    const statusRows = Array.isArray(data) ? data : [];
    if (!Array.isArray(data)) {
      console.warn('Status response is not an array. Rendering base status only.', data);
    }

    let html = `
      <h2>Current Status</h2>
      <table border="1" style="border-collapse: collapse; width: 100%;">
        <tr>
          <td style="padding: 10px; border: 1px solid #ccc;"><strong>Parameter</strong></td>
          <td style="padding: 10px; border: 1px solid #ccc;"><strong>Value</strong></td>
        </tr>
        <tr>
          <td style="padding: 10px; border: 1px solid #ccc;">WiFi Mode</td>
          <td style="padding: 10px; border: 1px solid #ccc;">${wifiMode}</td>
        </tr>
    `;

    statusRows.forEach(([parameter, value]) => {
      html += `
        <tr>
          <td style="padding: 10px; border: 1px solid #ccc;">${parameter}</td>
          <td style="padding: 10px; border: 1px solid #ccc;">${value}</td>
        </tr>
      `;
    });

    html += `</table>`;
    mainContent.innerHTML = html;
  } catch (error) {
    console.error('Error fetching data:', error);
    const message = error?.message || 'Unknown error';
    mainContent.innerHTML = `
      <p>Error loading data: ${message}.</p>
      <p>Connection tip: ensure this device is on the NodeMCU AP and try <a href="http://192.168.4.1/">http://192.168.4.1/</a>.</p>
    `;
  }
}

/**
 * Toggle the mobile navigation menu for the top navigation bar.
 */
function toggleMobileMenu() {
  const menu = document.getElementById('myTopnav');
  if (!menu) {
    return;
  }

  if (menu.className === 'topnav') {
    menu.className += ' responsive';
  } else {
    menu.className = 'topnav';
  }
}

async function saveMqttSettings(event) {
  event.preventDefault();

  const mqttEnabled = document.getElementById('mqttEnabled')?.checked || false;
  const mqttServer = document.getElementById('mqttServer')?.value?.trim() || '';
  const mqttPort = Number(document.getElementById('mqttPort')?.value || 1883);
  const mqttUser = document.getElementById('mqttUser')?.value?.trim() || '';
  const mqttPassword = document.getElementById('mqttPassword')?.value || '';
  const mqttTopic = document.getElementById('mqttTopic')?.value?.trim() || '';
  const mqttPublishInterval = Number(document.getElementById('mqttPublishInterval')?.value || 2000);

  const payload = {
    parameters: {
      mqtt_enabled: mqttEnabled,
      mqtt_server: mqttServer,
      mqtt_port: Number.isFinite(mqttPort) ? Math.max(1, Math.floor(mqttPort)) : 1883,
      mqtt_username: mqttUser,
      mqtt_password: mqttPassword,
      mqtt_base_topic: mqttTopic,
      mqtt_publish_interval_ms: Number.isFinite(mqttPublishInterval) ? Math.max(500, Math.floor(mqttPublishInterval)) : 2000
    }
  };

  try {
    const response = await apiFetch('/api/save', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(payload)
    });

    let result = {};
    try {
      result = await response.json();
    } catch {
      result = {};
    }

    if (!response.ok || result.success === false) {
      throw new Error(result.error || `Save failed (${response.status})`);
    }

    showMqttMessage('MQTT settings saved successfully.', 'info');
    await loadConfig();
    await fetchData();
  } catch (error) {
    showMqttMessage(error.message || 'Failed to save MQTT settings.', 'error');
  }
}

window.addEventListener('DOMContentLoaded', () => {
  fetchData();
  updateWifiRefreshState();

  window.addEventListener('hashchange', updateWifiRefreshState);

  const refreshButton = document.getElementById('refreshWifiList');
  if (refreshButton) {
    refreshButton.addEventListener('click', () => {
      if (isWifiSectionActive()) {
        loadWifiSsids();
      }
    });
  }

  const connectButton = document.getElementById('connectButton');
  if (connectButton) {
    connectButton.addEventListener('click', handleConnectAction);
  }

  const returnToApButton = document.getElementById('returnToApButton');
  if (returnToApButton) {
    returnToApButton.addEventListener('click', returnToApMode);
  }

  const cancelPassword = document.getElementById('cancelPassword');
  if (cancelPassword) {
    cancelPassword.addEventListener('click', closePasswordDialog);
  }

  const togglePasswordVisibility = document.getElementById('togglePasswordVisibility');
  if (togglePasswordVisibility) {
    togglePasswordVisibility.addEventListener('click', () => {
      const passwordInput = document.getElementById('ssid-password');
      const isVisible = passwordInput?.type === 'text';
      setPasswordVisibility(!isVisible);
      passwordInput?.focus();
    });
  }

  const submitPassword = document.getElementById('submitPassword');
  if (submitPassword) {
    submitPassword.addEventListener('click', () => {
      const passwordInput = document.getElementById('ssid-password');
      const network = getSelectedNetwork();
      if (!network) {
        setPasswordDialogError('No network selected.');
        return;
      }

      const password = passwordInput?.value?.trim() || '';
      if (password.length < 8) {
        setPasswordDialogError('Password must be at least 8 characters.');
        return;
      }

      closePasswordDialog();
      attemptConnect(network.ssid, password);
    });
  }

  const mqttForm = document.getElementById('mqttForm');
  if (mqttForm) {
    mqttForm.addEventListener('submit', saveMqttSettings);
  }
});
