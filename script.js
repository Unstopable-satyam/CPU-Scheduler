document.getElementById('addProcess').addEventListener('click', function() {
    const processDiv = document.createElement('div');
    processDiv.className = 'process';
    processDiv.innerHTML = `
        <input type="number" name="pid" placeholder="PID" required>
        <input type="number" name="burst_time" placeholder="Burst Time" required>
        <input type="number" name="arrival_time" placeholder="Arrival Time" required>
    `;
    document.getElementById('processes').appendChild(processDiv);
});

document.getElementById('processForm').addEventListener('submit', function(e) {
    e.preventDefault();

    const formData = new FormData(e.target);
    const processes = [];
    for (let i = 0; i < formData.getAll('pid').length; i++) {
        processes.push({
            pid: formData.getAll('pid')[i],
            burst_time: formData.getAll('burst_time')[i],
            arrival_time: formData.getAll('arrival_time')[i]
        });
    }

    // const quantum = formData.get('time_quantum');

    fetch('/schedule', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ processes })
    })
    .then(response => {
        if (!response.ok) {
            return response.text().then(text => { throw new Error(text) });
        }
        return response.json();
    })
    .then(data => {
        document.getElementById('output').textContent = data.result;
    })
    .catch(error => {
        console.error('Error:', error);
        document.getElementById('output').textContent = 'Error: ' + error.message;
    });
});
