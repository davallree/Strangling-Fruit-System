export class PressedSound {
  speedMultiplier = 0;

  constructor() {
    this.reverb = new Tone.Reverb(0.5).toDestination();
    this.pressedSynth = new Tone.FMSynth().toDestination();
    this.bassSynth = new Tone.MonoSynth({
      oscillator: { type: "sine" },
      envelope: { attack: 0.001, decay: 0, sustain: 1, release: 10 }
    }).toDestination();
    this.notesForMultiplier = [["C2", "D4"], ["F2", "G4"], ["Bb3", "C5"], ["Eb3", "F5"]];
    // this.bassOscillator = new Tone.Oscillator("C2", "sine").toDestination();

    this.bassGain = new Tone.Gain(0).toDestination();
    this.bassOscillator = new Tone.Oscillator({
      frequency: "C1",
      type: "sine"
    }).start();
    this.bassOscillator.connect(this.bassGain);

    this.lfo = new Tone.LFO({
      frequency: this.speedMultiplier,
      min: 0,
      max: 1
    }).start();
    this.lfo.connect(this.bassGain.gain);

    // A soft chime sound that accelerates over time and becomes more distorted.
    this.chebyshev = new Tone.Chebyshev(50).toDestination();
    this.acceleratingSynth = new Tone.Synth({
      oscillator: {
        type: 'sine' // Use a sine wave for a soft sound
      },
      envelope: {
        attack: 0.001,
        decay: 0.2,
        sustain: 0.1,
        release: 1.5,
      },
    }).connect(this.chebyshev);
    this.acceleratingSynthLoop = new Tone.Loop((time) => {
      this.acceleratingSynth.triggerAttackRelease("C6", "8n", time);
    }, "4n");

    this.pressedSynth.connect(this.reverb);
    this.bassSynth.connect(this.reverb);
    this.acceleratingSynth.connect(this.reverb);
  }

  play() {
    Tone.Transport.bpm.value = 60;
    Tone.Transport.start();
    this.bassSynth.triggerAttackRelease(this.notesForMultiplier[this.speedMultiplier][0], "8n");
    this.pressedSynth.triggerAttackRelease(this.notesForMultiplier[this.speedMultiplier][1], "8n");

    this.chebyshev.wet.value = 0;
    this.acceleratingSynth.detune.value = 0;
    this.acceleratingSynthLoop.start("+5");
    Tone.Transport.bpm.exponentialRampTo(800, 10, "+5");
    this.acceleratingSynth.detune.exponentialRampTo(1200, 14, "+1");
    this.chebyshev.wet.rampTo(1, 5, "+10");
  }

  pause() {
    this.bassGain.gain.linearRampToValueAtTime(0, Tone.now() + 1);
    this.lfo.stop();
    this.acceleratingSynthLoop.stop();
    Tone.getTransport().bpm.value = 60;
    Tone.Transport.stop();
  }
}

