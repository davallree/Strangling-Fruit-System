import '../Tone.js';

// This is a one-shot sound that plays a random Brandy sample.
export class BrandySound {
  constructor() {
    this.players = [];
    // Iterate through the files in sounds/Brandy and add them to the players.
    for (let i = 1; i <= 27; i++) {
      const player = new Tone.Player(`../sounds/Brandy/Brandy-${i}.mp3`).toDestination();
      this.players.push(player);
    }
  }

  play() {
    Tone.Transport.stop().start();
    // Play a random sound.
    const soundIndex = Math.floor(Math.random() * 27) + 1;
    this.players[soundIndex].start();
  }
}
