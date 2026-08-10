let currentConfig = null;
let ssidRefreshTimer = null;

/**
 * Load device configuration from the mock API and update the page title.
 */
async function loadConfig() {
  try {
    const response = await fetch('/api/config');
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
  errorElement.textContent = '';
  errorElement.classList.add('hidden');
  dialog.classList.remove('hidden');
  passwordInput.focus();
}

function closePasswordDialog() {
  const dialog = document.getElementById('password-dialog');
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
  clearWifiMessage();
  showWifiMessage(`Connecting to ${ssid}...`, 'info');

  try {
    const response = await fetch('/api/connect', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ ssid, password })
    });

    const result = await response.json();
    if (!response.ok) {
      throw new Error(result.error || 'Failed to connect');
    }

    showWifiMessage(result.message || `Connected to ${ssid}.`, 'info');
  } catch (error) {
    showWifiMessage(error.message || 'Connection failed.', 'error');
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

    const response = await fetch('/api/ssids');
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
    const response = await fetch('/api/status');
    const data = await response.json();

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

    data.forEach(([parameter, value]) => {
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
    mainContent.innerHTML = '<p>Error loading data.</p>';
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

  const cancelPassword = document.getElementById('cancelPassword');
  if (cancelPassword) {
    cancelPassword.addEventListener('click', closePasswordDialog);
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
});
