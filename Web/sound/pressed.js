class PressedSound {
  speedMultiplier = 0;

  constructor(masterLimiter) {
    this.pressedSynth = new Tone.FMSynth().connect(masterLimiter);
    this.pressedSynth.volume.value = 0;
    this.bassSynth = new Tone.MonoSynth({
      oscillator: { type: "sine" },
      envelope: { attack: 0, decay: 0, sustain: 1, release: 10 }
    }).connect(masterLimiter);
    this.bassSynth.volume.value = 0;
    this.notesForMultiplier = [["C2", "D4"], ["F2", "G4"], ["Bb3", "C5"], ["Eb3", "F5"]];
    // this.bassOscillator = new Tone.Oscillator("C2", "sine").toDestination();
  }

  play() {
    this.bassSynth.triggerAttackRelease(this.notesForMultiplier[this.speedMultiplier][0], "4n");
    this.pressedSynth.triggerAttackRelease(this.notesForMultiplier[this.speedMultiplier][1], "4n");
    // bassOscillator.frequency.value = 220 * speedMultiplier;  // 220 Hz is A2
    // bassOscillator.start();
    // setTimeout(() => bassOscillator.stop(), 500);  // Plays for 500 ms
  }

  pause() {
    // Nothing to pause.
  }
}

