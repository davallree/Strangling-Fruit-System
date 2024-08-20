import './Tone.js';
import { SerialHandler } from './serial.js';
import { AmbientSound } from './sound/ambient.js';
import { PressedSound } from './sound/pressed.js';
import { GlitchSound } from './sound/glitch.js';
import { DullSound } from './sound/dull.js';
import { ClimaxSound } from './sound/climax.js';

class CubeApp {
  numWalls = 4;

  serialHandler = new SerialHandler();
  connectButton = document.getElementById('connect-button');

  masterStatusIndicator = document.getElementById('master-status');

  restartMasterButton = document.getElementById('restart-master-button');

  messagesConsole = document.getElementById('messages');
  currentSound = null;

  ambientButton = document.getElementById('ambient-button');
  glitchedButton = document.getElementById('glitched-button');
  climaxButton = document.getElementById('climax-button');
  pressed1Button = document.getElementById('pressed1-button');
  pressed2Button = document.getElementById('pressed2-button');
  pressed3Button = document.getElementById('pressed3-button');
  pressed4Button = document.getElementById('pressed4-button');

  testMessageButton = document.getElementById('test-message-button');

  meter = new Tone.Meter();

  constructor() {
    this.setMasterConnected(false);
    this.wallStatusIndicators = Array.from({ length: this.numWalls }, (_, i) =>
      document.getElementById(`wall${i}-status`)
    );
    this.wallAddresses = Array.from({ length: this.numWalls }, (_, i) =>
      document.getElementById(`wall${i}-address`)
    );
    for (let i = 0; i < this.numWalls; i++) {
      document.getElementById(`restart-wall${i}-button`)
        .addEventListener('pointerdown', () => this.sendRestartWallMessage(i));
      document.getElementById(`set-threshold-wall${i}`).addEventListener('pointerdown', () => this.sendSetTouchThresholdMessage(i));
      this.setWallStatus(i, { lastDeliveryStatus: "unknown" });
    }
    this.connectButton.addEventListener('pointerdown', this.connect);
    this.restartMasterButton.addEventListener('pointerdown', this.sendRestartMasterMessage);

    document.getElementById('set-normal')
      .addEventListener('pointerdown', () => this.sendSetCubeModeMessage('normal'));
    document.getElementById('set-man-burn')
      .addEventListener('pointerdown', () => this.sendSetCubeModeMessage('manBurn'));
    document.getElementById('set-temple-burn')
      .addEventListener('pointerdown', () => this.sendSetCubeModeMessage('templeBurn'));

    this.serialHandler.messageCallback = this.onMessage;

    this.ambientSound = new AmbientSound();
    this.pressedSound = new PressedSound();
    this.glitchSound = new GlitchSound();
    this.dullSound = new DullSound();
    this.climaxSound = new ClimaxSound();

    // Reduce the volume so we don't get clipping.
    Tone.Master.volume.value = -10;

    // Wire up test buttons.
    this.ambientButton.addEventListener('pointerdown', () => this.playSound('ambient'));
    this.glitchedButton.addEventListener('pointerdown', () => this.playSound('glitch'));
    this.climaxButton.addEventListener('pointerdown', () => this.playSound('climax'));
    this.pressed1Button.addEventListener('pointerdown', () => this.playSound('pressed', { pressedCount: 1 }));
    this.pressed2Button.addEventListener('pointerdown', () => this.playSound('pressed', { pressedCount: 2 }));
    this.pressed3Button.addEventListener('pointerdown', () => this.playSound('pressed', { pressedCount: 3 }));
    this.pressed4Button.addEventListener('pointerdown', () => this.playSound('pressed', { pressedCount: 4 }));

    this.testMessageButton.addEventListener('pointerdown', async () => {
      await this.serialHandler.send('debug', ['Test message']);
    });

    Tone.getDestination().connect(this.meter);
    this.updateMeter();
  }

  setMasterConnected = (connected) => {
    if (connected) {
      this.masterStatusIndicator.style.backgroundColor = 'green';
    } else {
      this.masterStatusIndicator.style.backgroundColor = 'red';
    }
  }

  setWallStatus = (id, status) => {
    const indicator = this.wallStatusIndicators[id];
    if (status.lastDeliveryStatus === "success") {
      indicator.style.backgroundColor = 'green';
    } else if (status.lastDeliveryStatus === "failure") {
      indicator.style.backgroundColor = 'red';
    } else {
      indicator.style.backgroundColor = 'yellow';
    }
  }

  updateMeter = () => {
    const level = this.meter.getValue();
    const normalizedLevel = Tone.dbToGain(level);
    const meterElement = document.getElementById('meter-level');
    meterElement.style.width = Math.min(normalizedLevel * 100, 100) + "%";
    requestAnimationFrame(this.updateMeter);
  }

  // Connect to the cube, and start Tone.js.
  connect = async () => {
    await Tone.start();
    // Loads the samples.
    await Tone.loaded();
    try {
      await this.serialHandler.connect();
      console.log('Connected to device');
      this.setMasterConnected(true);
      await this.serialHandler.read();
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
      case 'updateStatus':
        this.updateStatus(msg.params);
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
    console.log('playing sound: ', soundName, soundParams);
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

  sendRestartMasterMessage = async () => {
    await this.serialHandler.send('restartMaster', []);
  }

  sendRestartWallMessage = async (wallId) => {
    await this.serialHandler.send('restartWall', { 'wallId': wallId })
  }

  sendSetTouchThresholdMessage = async (wallId) => {
    const touchThreshold = document.getElementById(`threshold-wall${wallId}-input`).value;
    await this.serialHandler.send(
      'setTouchThreshold', { 'wallId': wallId, 'touchThreshold': touchThreshold });
  }

  sendSetCubeModeMessage = async (cubeMode) => {
    await this.serialHandler.send('setCubeMode', { 'cubeMode': cubeMode })
  }

  updateStatus = (params) => {
    params.walls.forEach((status, index) => {
      this.setWallStatus(index, status);
      this.wallAddresses[index].textContent = status.address;
    });
  }

}
export const app = new CubeApp();
