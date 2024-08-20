import "../Tone.js";

export class PressedSound {
  _speedMultiplier = 0;
  _increasing = false;

  set speedMultiplier(numPressed) {
    this._increasing = numPressed > this._speedMultiplier;
    this._speedMultiplier = numPressed;
  }

  get speedMultiplier() {
    return this._speedMultiplier;
  }

  constructor() {
    this.compressor = new Tone.Compressor().toDestination();
    this.pressedSynth = new Tone.FMSynth({ volume: -10 }).connect(this.compressor);
    this.bassSynth = new Tone.MonoSynth({
      volume: -10,
      oscillator: { type: "sine" },
      envelope: { attack: 0.001, decay: 0, sustain: 1, release: 10 }
    }).connect(this.compressor);
    this.notesForMultiplier = [["C2", "D4"], ["F2", "G4"], ["Bb3", "C5"], ["Eb3", "F5"], ["Ab3", "Bb5"]];

    this.bassOscillator = new Tone.Oscillator({
      frequency: "G2",
      type: "sine",
      volume: -10,
    });
    this.bassGain = new Tone.Gain(0).connect(this.compressor);
    this.bassOscillator.connect(this.bassGain);

    this.bassLfo = new Tone.LFO({
      frequency: this.speedMultiplier,
      min: 0,
      max: 1
    });
    this.bassLfo.connect(this.bassGain.gain);

    // A soft chime sound that accelerates over time and becomes more distorted.
    this.acceleratingSynthGain = new Tone.Gain(0).connect(this.compressor);
    this.chebyshev = new Tone.Chebyshev(50).connect(this.acceleratingSynthGain);
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
  }

  play() {
    Tone.Transport.stop();
    Tone.Transport.start();
    Tone.Transport.bpm.value = 60;
    this.bassSynth.triggerAttackRelease(this.notesForMultiplier[this.speedMultiplier][0], "8n");

    if (this._increasing) {
      this.pressedSynth.triggerAttackRelease(this.notesForMultiplier[this.speedMultiplier][1], "8n");
    }

    // Start the bass LFO.
    this.bassOscillator.start();
    this.bassGain.gain.linearRampToValueAtTime(1, Tone.now() + 1);
    this.bassLfo.frequency.value = this.speedMultiplier;
    this.bassLfo.start();

    // Start the accelerating synth 5 seconds after the start.
    this.acceleratingSynthGain.gain.value = 0;
    this.acceleratingSynthGain.gain.rampTo(1, 10, "+5");
    this.chebyshev.wet.value = 0;
    this.acceleratingSynth.detune.value = 0;
    this.acceleratingSynthLoop.start("+5");
    Tone.Transport.bpm.exponentialRampTo(800, 10, "+5");
    this.acceleratingSynth.detune.exponentialRampTo(1200, 14, "+1");
    this.chebyshev.wet.rampTo(1, 5, "+10");
  }

  pause() {
    this.bassOscillator.stop();
    this.bassGain.gain.linearRampToValueAtTime(0, Tone.now() + 1);
    this.bassLfo.stop();
    this.acceleratingSynthLoop.cancel();
    Tone.getTransport().bpm.value = 60;
    Tone.Transport.stop();
    Tone.Transport.cancel();
  }
}

