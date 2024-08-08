import './Tone.js';
import { SerialHandler } from './serial.js';
import { AmbientSound } from './sound/ambient.js';
import { PressedSound } from './sound/pressed.js';
import { GlitchSound } from './sound/glitch.js';
import { DullSound } from './sound/dull.js';
import { ClimaxSound } from './sound/climax.js';

class CubeApp {
  serialHandler = new SerialHandler();
  connectButton = document.getElementById('connect-button');
  messagesConsole = document.getElementById('messages');
  currentSound = null;

  constructor() {
    this.connectButton.addEventListener('pointerdown', this.connect);
    this.serialHandler.messageCallback = this.onMessage;

    this.ambientSound = new AmbientSound();
    this.pressedSound = new PressedSound();
    this.glitchSound = new GlitchSound();
    this.dullSound = new DullSound();
    this.climaxSound = new ClimaxSound();

    // Reduce the volume so we don't get clipping.
    Tone.Master.volume.value = -10;
  }

  // Connect to the cube, and start Tone.js.
  connect = async () => {
    await Tone.start();
    // Loads the samples.
    await Tone.loaded();
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
      case 'playOneShot':
        this.playOneShot(msg.params.soundName, msg.params.soundParams);
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
      case 'climax':
        this.currentSound = this.climaxSound;
        break;
    }
    this.currentSound.play();
  }

  playOneShot(soundName, soundParams) {
    // One shot sounds do not pause the current sound.
    console.log('playing one shot: ', soundName);
    switch (soundName) {
      case 'dull':
        this.dullSound.play();
        break;
    }
  }
}
export const app = new CubeApp();
