import "../Tone.js";

export class ClimaxSound {
  constructor() {
    // Compressor to keep levels in check.
    this.compressor = new Tone.Compressor().toDestination();

    // Create a gain envelope to control the rise
    this.gainEnvelope = new Tone.Gain(0).connect(this.compressor);

    // Create a group to hold all oscillators
    this.synthGroup = new Tone.Gain(1).connect(this.gainEnvelope);

    // Bring back the synths from the pressed sound.
    this.pressedSynth = new Tone.PolySynth(Tone.FMSynth, { volume: -10 }).connect(this.compressor);
    this.bassSynth = new Tone.PolySynth(Tone.MonoSynth, {
      oscillator: { type: "sine" },
      envelope: { attack: 0.001, decay: 0, sustain: 1, release: 10 }
    }).connect(this.compressor);

    // Create reverb and delay effects
    this.reverb = new Tone.Reverb({
      decay: 4,
      wet: 0.5
    }).connect(this.gainEnvelope);
    this.delay = new Tone.FeedbackDelay({
      delayTime: 0.25,
      feedback: 0.6,
      wet: 0.4
    }).connect(this.gainEnvelope);

    // Connect synth group to effects
    this.synthGroup.connect(this.reverb);
    this.synthGroup.connect(this.delay);

    // Create multiple detuned oscillators
    this.oscillators = [];
    for (let i = 0; i < 8; i++) {
      const osc = new Tone.Oscillator({
        frequency: 100,
        type: "sawtooth",
        detune: (i - 4) * ((Math.random() * 100) - 50)
      }).connect(this.synthGroup);
      this.oscillators.push(osc);
    }
  }

  play() {
    // Play the pressed notes all together.
    this.pressedSynth.triggerAttackRelease(["D4", "G4", "C5", "F5", "Bb5"], "1m");
    this.bassSynth.triggerAttackRelease(["C2", "F2", "Bb3", "Eb3", "Ab3"], "1m");
    this.gainEnvelope.gain.value = 0;
    this.oscillators.forEach(osc => { osc.frequency.value = 100; osc.start(); });
    const riseTime = 20;
    // Create an automation for volume rise
    this.gainEnvelope.gain.rampTo(1, riseTime);

    // Create a frequency rise for each oscillator
    this.oscillators.forEach((osc, index) => {
      osc.frequency.linearRampTo(((Math.random() * 1000) + 50), riseTime);
    });

    // Stop oscillators after the rise
    setTimeout(() => {
      this.oscillators.forEach(osc => osc.stop());
      console.log('Oscillators stopped');
    }, riseTime * 1000);
  }

  pause() {
    this.oscillators.forEach(osc => osc.stop());
  }
}
