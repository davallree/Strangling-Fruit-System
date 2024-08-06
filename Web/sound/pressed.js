class PressedSound {
  speedMultiplier = 0;

  constructor(masterLimiter) {
    this.basicGain = new Tone.Gain(1).connect(masterLimiter);
    this.pressedSynth = new Tone.FMSynth().connect(this.basicGain);
    this.pressedSynth.volume.value = 0;
    this.bassSynth = new Tone.MonoSynth({
      oscillator: { type: "sine" },
      envelope: { attack: 0.001, decay: 0, sustain: 1, release: 10 }
    }).connect(this.basicGain);
    this.bassSynth.volume.value = 0;
    this.notesForMultiplier = [["C2", "D4"], ["F2", "G4"], ["Bb3", "C5"], ["Eb3", "F5"]];
    // this.bassOscillator = new Tone.Oscillator("C2", "sine").toDestination();

    this.bassOscillator = new Tone.Oscillator({
      frequency: "C1",
      type: "sine"
    }).start();
    this.bassGain = new Tone.Gain(0).connect(masterLimiter);
    this.bassOscillator.connect(this.bassGain);

    this.lfo = new Tone.LFO({
      frequency: this.speedMultiplier,
      min: 0,
      max: 1
    }).start();
    this.lfo.connect(this.bassGain.gain);

    // A soft chime sound that accelerates over time.
    this.acceleratingSynth = new Tone.Synth({
      oscillator: {
        type: 'sine' // Use a sine wave for a soft sound
      },
      envelope: {
        attack: 0.001,
        decay: 0.2,
        sustain: 0.1,
        release: 1.5
      },
      volume: -20,
    }).connect(this.basicGain);
    this.acceleratingSynthLoop = new Tone.Loop((time) => {
      this.acceleratingSynth.triggerAttackRelease("C6", "8n", time);
    }, "8n");
  }

  play() {
    this.bassSynth.triggerAttackRelease(this.notesForMultiplier[this.speedMultiplier][0], "4n");
    this.pressedSynth.triggerAttackRelease(this.notesForMultiplier[this.speedMultiplier][1], "4n");
    this.acceleratingSynthLoop.start();
    Tone.getTransport().bpm.value = 60;
    Tone.getTransport().bpm.rampTo(800, 5);


    // this.bassGain.gain.setValueAtTime(1, Tone.now());
    // this.lfo.frequency.value = this.speedMultiplier;
    // this.lfo.start();
    // bassOscillator.frequency.value = 220 * speedMultiplier;  // 220 Hz is A2
    // bassOscillator.start();
    // setTimeout(() => bassOscillator.stop(), 500);  // Plays for 500 ms
  }

  pause() {
    this.bassGain.gain.linearRampToValueAtTime(0, Tone.now() + 1);
    this.lfo.stop();
    this.acceleratingSynthLoop.stop();
    Tone.getTransport().bpm.value = 60;
    // Nothing to pause.
  }
}

