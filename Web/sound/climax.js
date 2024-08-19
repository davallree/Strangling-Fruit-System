import "../Tone.js";

export class ClimaxSound {
  constructor() {
    // Compressor to keep levels in check.
    this.compressor = new Tone.Compressor().toDestination();

    // Create a gain envelope to control the synth volume rise.
    this.gainEnvelope = new Tone.Gain(0).connect(this.compressor);

    // Create a group to hold all oscillators
    this.synthGroup = new Tone.Gain(1).connect(this.gainEnvelope);

    // Bring back the synths from the pressed sound.
    this.pressedSynth = new Tone.PolySynth(Tone.FMSynth, { volume: -10 }).connect(this.compressor);
    this.bassSynth = new Tone.PolySynth(Tone.MonoSynth, {
      volume: -10,
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
    // Reset oscillators.
    this.oscillators.forEach(osc => osc.frequency.value = 100);
    // Start the transport from the beginning.
    Tone.Transport.stop().start();
    // Set the BPM.
    Tone.Transport.bpm.value = 80;

    // Play the pressed notes all together for one measure.
    const seq = new Tone.Sequence((time, note) => {
      this.pressedSynth.triggerAttack(note, time);
    }, ["G4", "C5", "F5", "Bb5"], "16n");
    seq.loop = false;
    seq.start("0:0:0");

    const bassSeq = new Tone.Sequence((time, note) => {
      this.bassSynth.triggerAttack(note, time);
    }, ["F2", "Bb3", "Eb3", "Ab3"], "16n");
    bassSeq.loop = false;
    bassSeq.start("0:0:0");

    // Release halfways through the second measure.
    Tone.Transport.scheduleOnce((time) => {
      this.pressedSynth.releaseAll(time);
      this.bassSynth.releaseAll(time);
    }, "1:2:0");

    // Reset the oscillators gain to 0.
    this.gainEnvelope.gain.value = 0;
    const riseTime = 20;
    // Start the oscillators at the second measure.
    Tone.Transport.scheduleOnce((time) => {
      // Reset oscillators and ramp them up.
      this.oscillators.forEach(osc => {
        // Start the oscillator at 100Hz and ramp it up to a random frequency.
        // (Each oscillator is detuned by a random amount.)
        osc.frequency.value = 100;
        osc.start(time);
        osc.frequency.linearRampTo(((Math.random() * 1000) + 50), riseTime, time);
      });
      // Create an automation for volume rise.
      this.gainEnvelope.gain.rampTo(1, riseTime);
    }, "1:0:0");

    // Stop oscillators after the rise
    Tone.Transport.scheduleOnce((time) => {
      this.oscillators.forEach(osc => osc.stop(time));
      console.log('Oscillators stopped');
    }, `+${riseTime}`);
  }

  pause() {
    // Stop transport and remove all events.
    Tone.Transport.stop().cancel();
    // Silence the synths and oscillators.
    this.pressedSynth.releaseAll();
    this.bassSynth.releaseAll();
    this.oscillators.forEach(osc => osc.stop());
  }
}
