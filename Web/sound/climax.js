import "../Tone.js";

export class ClimaxSound {
  constructor() {
    // Create a gain envelope to control the rise
    this.gainEnvelope = new Tone.Gain(0).toDestination();

    // Create a group to hold all oscillators
    this.synthGroup = new Tone.Compressor().connect(this.gainEnvelope);

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
    this.oscillators.forEach(osc => osc.start());
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
