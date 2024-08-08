export class GlitchSound {
  static async create(masterLimiter) {
    const sound = new GlitchSound(masterLimiter);
    await sound.load();
    return sound;
  }

  constructor(masterLimiter) {
    this.player = new Tone.Player().connect(masterLimiter);
    this.player.loop = true;
  }

  async load() {
    await this.player.load("./sounds/glitch.mp3");
  }

  play() {
    Tone.Transport.start();
    console.log("Playing glitch sound.");
    this.player.start();
    this.jumpLoop = new Tone.Loop((time) => {
      const randomPosition = Math.random() * this.player.buffer.duration;
      this.player.seek(randomPosition, time);
    }, "16n");
    this.jumpLoop.start();
  }

  pause() {
    this.player.stop();
    this.jumpLoop.stop();
  }
}
