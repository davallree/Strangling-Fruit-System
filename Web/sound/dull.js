import "../Tone.js";

// A dull sound for when the hands are deactivated.
export class DullSound {
  constructor() {
    this.synth = new Tone.DuoSynth({
      harmonicity: 1.1,
      voice0: {
        oscillator: {
          type: 'sine'
        },
        envelope: {
          attack: 0.1,
          decay: 0,
          sustain: 1,
          release: 0.1
        }
      },
      voice1: {
        oscillator: {
          type: 'sine'
        },
        envelope: {
          attack: 0.01,
          decay: 0,
          sustain: 1,
          release: 0.1
        }
      }
    });
    this.reverb = new Tone.Reverb({ decay: 1, wet: 0.5 }).toDestination();
    this.synth.connect(this.reverb);
  }

  play() {
    this.synth.triggerAttackRelease("C3", "8n");
  }

  pause() {
    // Nothing to pause.
  }
}
