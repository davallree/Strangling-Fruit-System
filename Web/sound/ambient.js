export class AmbientSound {
  playing = false;
  constructor() {
    // Ambient sounds setup
    this.masterCompressor = new Tone.Compressor().toDestination();
    this.masterReverb = new Tone.Reverb({ decay: 20, wet: 0.7 }).connect(this.masterCompressor);
    this.masterDelay = new Tone.FeedbackDelay({ delayTime: 0.2, feedback: 0.2 }).connect(this.masterReverb);
    this.masterFilter = new Tone.Filter({ type: "lowpass", frequency: 1000, rolloff: -24 }).connect(this.masterDelay);

    // Deep bass drone
    this.bassDrone = new Tone.FatOscillator({
      type: "sine",
      frequency: "A0",
      spread: 60,
      count: 3
    }).connect(this.masterFilter);
    this.bassDrone.volume.value = 0; // Set volume to a reasonable level

    // Mysterious pad with increased polyphony
    this.pad = new Tone.PolySynth(Tone.Synth, {
      oscillator: { type: "triangle" },
      envelope: { attack: 2, decay: 1, sustain: 0.8, release: 5 },
      maxPolyphony: 16
    }).connect(this.masterFilter);
    this.pad.volume.value = 0; // Set volume to a reasonable level

    // Ethereal texture
    this.texture = new Tone.Noise({ type: "brown" })
      .connect(this.masterFilter)
      .start();
    this.textureFilter = new Tone.AutoFilter({ frequency: 0.1, octaves: 5, baseFrequency: 100 })
      .connect(this.texture.volume)
      .start();
    this.texture.volume.value = -40; // Initial volume of texture

    // Accent synth
    this.accentSynth = new Tone.MembraneSynth({
      pitchDecay: 0.05,
      octaves: 2,
      oscillator: { type: "sine" },
      envelope: {
        attack: 0.01,
        decay: 0.4,
        sustain: 0.01,
        release: 0.4,
        attackCurve: "exponential"
      }
    }).connect(this.masterFilter);
    this.accentSynth.volume.value = -7; // Increase accent synth volume

    // LFOs for various modulations
    this.bassLFO = new Tone.LFO({ frequency: 0.03, min: -30, max: 30 })
      .connect(this.bassDrone.frequency)
      .start();
    // Seems like pad LFO was never connected.
    this.padLFO = new Tone.LFO({ frequency: 0.09, min: -5, max: 5 });
    this.padLFO.start();
    this.filterLFO = new Tone.LFO({ frequency: 0.02, min: 100, max: 1000 })
      .connect(this.masterFilter.frequency)
      .start();

    // Panner for spatial effects
    this.panner = new Tone.Panner().connect(this.masterFilter);
    this.bassDrone.connect(this.panner);
    this.pad.connect(this.panner);
  }

  // Function to generate deep pad chords
  playPadChord(now) {
    const root = Tone.Frequency("A0").toFrequency();
    const chord = [
      root,
      root * Tone.intervalToFrequencyRatio(Math.random() < 0.5 ? 3 : 4),
      root * Tone.intervalToFrequencyRatio(7)
    ];
    this.pad.triggerAttackRelease(chord, "4n", now + Math.random() * 10);
    console.log("Pad chord played:", chord);
  }

  // Function to play accent sounds
  playAccent(now) {
    const duration = Math.random() * 0.6 + 0.2;
    const note = Tone.Frequency(Math.random() * 12 + 25, "midi").toFrequency();

    // this.accentSynth.volume.setValueAtTime(-5, now);
    this.accentSynth.triggerAttackRelease(note, duration, now);

    // Random panning for each accent
    this.panner.pan.setValueAtTime(Math.random() * 2 - 1, now);

    // Subtle filter sweep for added texture
    const filterSweep = new Tone.Filter({
      type: "lowpass",
      frequency: note * 2,
      Q: 1
    }).connect(this.masterFilter);

    filterSweep.frequency.linearRampToValueAtTime(note / 2, now + duration);
    this.accentSynth.connect(filterSweep);

    // Disconnect after the sound is finished
    Tone.Transport.scheduleOnce(() => {
      this.accentSynth.disconnect(filterSweep);
      filterSweep.dispose();
    }, `+${duration + 0.1}`);

    console.log("Accent note played:", note, "duration:", duration);
  }

  // Function to start/stop the bass drone
  toggleBassDrone(now) {
    if (this.bassDrone.state === "started") {
      this.bassDrone.stop(now);
      console.log("Bass drone stopped.");
    } else {
      this.bassDrone.start(now);
      console.log("Bass drone started.");
    }
  }

  // Function to update effects and modulations
  updateEffects(now) {
    this.panner.pan.linearRampToValueAtTime(Math.random() * 2 - 1, now + 20);
    this.masterFilter.frequency.linearRampToValueAtTime(Math.random() * 1500 + 80, now + 30);
    this.bassDrone.spread = Math.random() * 80 + 20;

    this.bassLFO.frequency.linearRampToValueAtTime(Math.random() * 0.05, now + 20);
    this.padLFO.frequency.linearRampToValueAtTime(Math.random() * 0.08, now + 20);
    this.filterLFO.frequency.linearRampToValueAtTime(Math.random() * 0.03, now + 20);
    this.textureFilter.baseFrequency = Math.random() * 200 + 50;

    const bassVolume = Math.max(Math.random() * 30 - 30, -20);
    this.bassDrone.volume.linearRampToValueAtTime(bassVolume, now + Math.random() * 20 + 10);

    const textureVolume = Math.random() * 20 - 80;
    this.texture.volume.linearRampToValueAtTime(textureVolume, now + Math.random() * 60 + 30);

    console.log("Effects updated");
  }

  // Schedule random events with varying intervals
  scheduleEvents() {
    console.log("Scheduling events.");
    const nextPadTime = Math.random() * 10 + 10;
    const nextAccentTime = Math.random() * 5 + 5;
    const nextEffectTime = Math.random() * 10 + 10;
    const nextBassDrone = Math.random() * 60 + 5;

    Tone.Transport.scheduleOnce((time) => {
      this.playPadChord(time);
    }, `+${nextPadTime}`);

    Tone.Transport.scheduleOnce((time) => {
      this.playAccent(time);
    }, `+${nextAccentTime}`);

    Tone.Transport.scheduleOnce((time) => {
      this.updateEffects(time);
    }, `+${nextEffectTime}`);

    Tone.Transport.scheduleOnce((time) => {
      this.toggleBassDrone(time);
    }, `+${nextBassDrone}`);

    Tone.Transport.scheduleOnce((time) => {
      this.scheduleEvents(time);
    }, `+${Math.max(nextPadTime, nextAccentTime, nextEffectTime) + 1}`);
  }

  play() {
    if (this.playing) return;
    this.playing = true;
    // Make sure the transport is started.
    Tone.Transport.stop();
    Tone.Transport.start();
    // Always start the bass drone and play a single accent note when we begin.
    this.bassDrone.start();
    this.playAccent(Tone.now());
    // Schedule the initial events.
    this.scheduleEvents();
    console.log("Ambient Sound is playing");
  }

  pause() {
    if (!this.playing) return;
    this.playing = false;
    // Stop the drone if it is playing.
    this.bassDrone.stop();
    // Clear out the scheduled events.
    Tone.Transport.cancel();
    console.log("Ambient Sound stopped");
  }
}
