async function fetchDevices() {
    const res = await fetch('/api/devices');
    const devices = await res.json();
    const container = document.getElementById('devices');
    container.innerHTML = '';

    devices.forEach(d => {
        const card = document.createElement('div');
        card.className = 'device-card' + (d.status === 'offline' ? ' offline' : '');
        card.innerHTML = `
            <span>${d.name} (${d.type}) — ${d.status}</span>
            <button data-mac="${d.mac}" data-cmd="toggle">Toggle</button>
            <button data-mac="${d.mac}" data-cmd="disconnect">Disconnect</button>
        `;
        container.appendChild(card);
    });

    container.querySelectorAll('button').forEach(btn => {
        btn.onclick = () => sendCommand(btn.dataset.mac, btn.dataset.cmd);
    });
}

async function sendCommand(mac, command) {
    await fetch('/api/command', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ mac, command })
    });
    setTimeout(fetchDevices, 300);
}

async function fetchPending() {
    const res = await fetch('/api/pending');
    const pending = await res.json();
    const container = document.getElementById('pending');
    container.innerHTML = '';

    pending.forEach(p => {
        const card = document.createElement('div');
        card.className = 'pending-card';
        card.innerHTML = `
            <span>${p.address}</span>
            <button data-addr="${p.address}">Подключить</button>
        `;
        container.appendChild(card);
    });

    container.querySelectorAll('button').forEach(btn => {
        btn.onclick = () => claimDevice(btn.dataset.addr);
    });
}

async function claimDevice(address) {
    const name = prompt('Название устройства:', 'Розетка');
    if (!name) return;
    await fetch('/api/claim', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ address, type: 'socket', name })
    });
    setTimeout(fetchDevices, 500);
}

document.getElementById('scanBtn').onclick = async () => {
    await fetch('/api/scan/start', { method: 'POST' });
};

setInterval(fetchDevices, 2000);
setInterval(fetchPending, 2000);
fetchDevices();
fetchPending();