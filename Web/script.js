class CubeApp {
  serialHandler = new SerialHandler();
  connectButton = document.getElementById('connect-button');
  messagesConsole = document.getElementById('messages');
  currentSound = null;

  constructor() {
    this.connectButton.addEventListener('pointerdown', this.connect);
    this.serialHandler.messageCallback = this.onMessage;

    this.masterLimiter = new Tone.Limiter(-3).toDestination();
    this.ambientSound = new AmbientSound(this.masterLimiter);
    this.pressedSound = new PressedSound(this.masterLimiter);
    this.glitchSound = new GlitchSound(this.masterLimiter);
  }

  // Connect to the cube, and start Tone.js.
  connect = async () => {
    await Tone.start();
    Tone.Transport.start();
    // Load the glitch sound here.
    this.glitchSound.load();
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
      case 'playSound':
        this.playSound(msg.params.soundName, msg.params.soundParams);
        break;
      default:
        console.error('unknown method: ', msg.method);
    }
  }

  debug(text) {
    const isScrolledToBottom = this.messagesConsole.scrollHeight - this.messagesConsole.clientHeight <= this.messagesConsole.scrollTop + 1;
    this.messagesConsole.innerHTML += `<p>[${new Date().toLocaleString()}] ${text}</p>`;
    if (isScrolledToBottom) {
      this.messagesConsole.scrollTop = this.messagesConsole.scrollHeight;
    }
  }

  playSound(soundName, soundParams) {
    if (this.currentSound) {
      this.currentSound.pause();
    }
    console.log('playing sound: ', soundName);
    switch (soundName) {
      case 'ambient':
        this.currentSound = this.ambientSound;
        break;
      case 'pressed':
        this.pressedSound.speedMultiplier = soundParams.pressedCount;
        this.currentSound = this.pressedSound;
        break;
      case 'glitch':
        this.currentSound = this.glitchSound;
        break;
    }
    this.currentSound.play();
  }
}
new CubeApp();
/*
let currentState = 'A';
let stateStartTime = null;
let glitchTimeout = null;

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
*/
