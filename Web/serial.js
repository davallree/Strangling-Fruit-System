let port;
let reader;
let inputDone;
let inputStream;

document.getElementById('connect-button').addEventListener('click', async () => {
    try {
        await connectSerial();
        console.log('Connected to device');
    } catch (error) {
        console.error('Failed to connect to device:', error);
    }
});

async function connectSerial() {
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 115200 });  // Ensure baud rate matches the ESP32 settings

    const decoder = new TextDecoderStream();
    inputDone = port.readable.pipeTo(decoder.writable);
    inputStream = decoder.readable;
    reader = inputStream.getReader();

    readLoop();
}

async function readLoop() {
    while (true) {
        try {
            const { value, done } = await reader.read();
            if (done) {
                console.log('[readLoop] DONE', done);
                reader.releaseLock();
                break;
            }
            if (value) {
                handleSerialData(value);
            }
        } catch (error) {
            console.error('Error reading data: ', error);
            break;
        }
    }
}

function handleSerialData(data) {
    // Clean up and split incoming data
    const lines = data.split('\n');
    for (const line of lines) {
        if (line.trim().length > 0) {
            console.log('Received:', line);
            const event = new CustomEvent('serialMessage', { detail: line.trim() });
            window.dispatchEvent(event);
        }
    }
}

window.connectToDevice = connectSerial;
