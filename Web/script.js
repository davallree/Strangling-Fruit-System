class CubeApp {
  serialHandler = new SerialHandler();
  connectButton = document.getElementById('connect-button');
  messagesConsole = document.getElementById('messages');

  constructor() {
    this.connectButton.addEventListener('pointerdown', this.connect);
    this.serialHandler.messageCallback = this.onMessage;
  }

  // Connect to the cube, and start Tone.js.
  connect = async () => {
    await Tone.start();
    try {
      await this.serialHandler.connect();
      console.log('Connected to device');
    } catch (error) {
      console.error('Failed to connect to device:', error);
    }
  }

  onMessage = (msg) => {
    console.log("message callback: ", msg);
    switch (msg.method) {
      case 'debug':
        this.debug(msg.params[0]);
        break;
      default:
        console.error('unknown method: ', msg.method);
    }
  }

  debug(text) {
    this.messagesConsole.innerHTML +=
      `<p>[${new Date().toLocaleString()}] ${text}</p>`;
  }

  playSound() {

  }
}
new CubeApp();
/*
document.getElementById('startButton').addEventListener('click', async () => {
  await Tone.start();
  document.getElementById('startButton').style.display = 'none';
  playAmbientSound();
  console.log("Web Audio Initialized and Ambient Sound Started");
});

let currentState = 'A';
let stateStartTime = null;
let glitchTimeout = null;

// Listen for incoming serial messages and handle them
window.addEventListener('serialMessage', (event) => {
  const message = event.detail;
  document.getElementById('messages').innerHTML += `<p>${message}</p>`;
  handleSerialMessage(message);
});

function handleSerialMessage(message) {
  const state = parseSerialMessage(message);
  if (state !== currentState) {
    transitionToState(state);
  }
}

function transitionToState(newState) {
  stopCurrentStateSound();

  if (['B', 'C', 'D'].includes(newState)) {
    stateStartTime = Date.now();
    glitchTimeout = setTimeout(() => {
      transitionToState('F');
    }, 15000); // 15 seconds to transition to glitch
  } else {
    stateStartTime = null;
    if (glitchTimeout) {
      clearTimeout(glitchTimeout);
      glitchTimeout = null;
    }
  }

  currentState = newState;
  startStateSound(newState);
}

function stopCurrentStateSound() {
  switch (currentState) {
    case 'A':
      stopAmbientSound();
      break;
    case 'B':
    case 'C':
    case 'D':
    case 'E':
      // No need to explicitly stop pressed sounds since they are short-lived
      break;
    case 'F':
      // Glitch sound stops automatically after playing
      break;
  }
}

function startStateSound(state) {
  switch (state) {
    case 'A':
      playAmbientSound();
      break;
    case 'B':
      playPressedSound(1);
      break;
    case 'C':
      playPressedSound(2);
      break;
    case 'D':
      playPressedSound(3);
      break;
    case 'E':
      playPressedSound(4);
      break;
    case 'F':
      playGlitchSound();
      break;
  }
}

function parseSerialMessage(message) {
  if (message.includes('0 button')) return 'A';
  if (message.includes('1 button')) return 'B';
  if (message.includes('2 button')) return 'C';
  if (message.includes('3 button')) return 'D';
  if (message.includes('4 button')) return 'E';
  return null;
}

window.handleSerialMessage = handleSerialMessage;
*/
