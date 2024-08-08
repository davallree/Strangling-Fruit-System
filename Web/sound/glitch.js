export class GlitchSound {
  constructor() {
    this.player = new Tone.Player("./sounds/glitch.mp3").toDestination();
    this.player.loop = true;
  }

  play() {
    Tone.Transport.stop();
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
