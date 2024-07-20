// Ambient sounds setup
const masterLimiter = new Tone.Limiter(-3).toDestination();
const masterCompressor = new Tone.Compressor({
    threshold: -15,
    ratio: 3,
    attack: 0.5,
    release: 0.1
}).connect(masterLimiter);
const masterReverb = new Tone.Reverb({ decay: 20, wet: 0.7 }).connect(masterCompressor);
const masterDelay = new Tone.FeedbackDelay({ delayTime: 0.2, feedback: 0.2 }).connect(masterReverb);
const masterFilter = new Tone.Filter({ type: "lowpass", frequency: 1000, rolloff: -24 }).connect(masterDelay);

// Simple oscillator test
const testOscillator = new Tone.Oscillator("C4", "sine").toDestination();

document.getElementById('startButton').addEventListener('click', async () => {
    await Tone.start();
    document.getElementById('startButton').style.display = 'none';
    testOscillator.start();
    setTimeout(() => {
        testOscillator.stop();
        playAmbientSound();
    }, 3000); // Play test tone for 3 seconds
    console.log("Test Oscillator playing");
});

// Deep bass drone
const bassDrone = new Tone.FatOscillator({
    type: "sine",
    frequency: "A0",
    spread: 60,
    count: 3
}).connect(masterFilter);
bassDrone.volume.value = 0; // Set volume to a reasonable level
console.log("bassDrone volume:", bassDrone.volume.value);

// Mysterious pad with increased polyphony
const pad = new Tone.PolySynth(Tone.Synth, {
    oscillator: { type: "triangle" },
    envelope: { attack: 2, decay: 1, sustain: 0.8, release: 5 },
    maxPolyphony: 16
}).connect(masterFilter);
pad.volume.value = 0; // Set volume to a reasonable level
console.log("pad volume:", pad.volume.value);

// Ethereal texture
const texture = new Tone.Noise({ type: "brown" }).connect(masterFilter);
const textureFilter = new Tone.AutoFilter({
    frequency: 0.1,
    octaves: 5,
    baseFrequency: 100
}).connect(texture.volume);
texture.volume.value = -40; // Initial volume of texture
texture.start();
console.log("texture volume:", texture.volume.value);

// Accent synth
const accentSynth = new Tone.MembraneSynth({
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
}).connect(masterFilter);
accentSynth.volume.value = -7; // Increase accent synth volume
console.log("accentSynth volume:", accentSynth.volume.value);

// LFOs for various modulations
const bassLFO = new Tone.LFO({ frequency: 0.03, min: -30, max: 30 }).connect(bassDrone.frequency);
const padLFO = new Tone.LFO({ frequency: 0.09, min: -5, max: 5 });
const filterLFO = new Tone.LFO({ frequency: 0.02, min: 100, max: 1000 }).connect(masterFilter.frequency);

// Panner for spatial effects
const panner = new Tone.Panner().connect(masterFilter);
bassDrone.connect(panner);
pad.connect(panner);

// Start continuous LFOs
bassLFO.start();
padLFO.start();
filterLFO.start();
textureFilter.start();

// Function to generate deep pad chords
function playPadChord() {
    const root = Tone.Frequency("A0").toFrequency();
    const chord = [
        root,
        root * Tone.intervalToFrequencyRatio(Math.random() < 0.5 ? 3 : 4),
        root * Tone.intervalToFrequencyRatio(7)
    ];
    pad.triggerAttackRelease(chord, "4n", Tone.now() + Math.random() * 10);
    console.log("Pad chord played:", chord);
}

// Function to play accent sounds
function playAccent() {
    const now = Tone.now();
    const duration = Math.random() * 0.6 + 0.2;
    const note = Tone.Frequency(Math.random() * 12 + 25, "midi").toFrequency();
    
    accentSynth.volume.setValueAtTime(-5, now);
    accentSynth.triggerAttackRelease(note, duration, now);

    // Random panning for each accent
    panner.pan.setValueAtTime(Math.random() * 2 - 1, now);

    // Subtle filter sweep for added texture
    const filterSweep = new Tone.Filter({
        type: "lowpass",
        frequency: note * 2,
        Q: 1
    }).connect(masterFilter);

    filterSweep.frequency.linearRampToValueAtTime(note / 2, now + duration);
    accentSynth.connect(filterSweep);

    // Disconnect after the sound is finished
    Tone.Transport.scheduleOnce(() => {
        accentSynth.disconnect(filterSweep);
        filterSweep.dispose();
    }, `+${duration + 0.1}`);

    console.log("Accent note played:", note, "duration:", duration);
}

// Function to randomly start and stop the bass drone
function toggleBassDrone() {
    const now = Tone.now();
    const nextStartTime = Math.random() * 20 + 5;
    const nextStopTime = Math.random() * 20 + 5;

    Tone.Transport.scheduleOnce(() => {
        bassDrone.start(now);
        console.log("Bass drone started");

        Tone.Transport.scheduleOnce(() => {
            bassDrone.stop(now + nextStopTime);
            toggleBassDrone();
            console.log("Bass drone stopped");
        }, now + nextStopTime);
    }, now + nextStartTime);
}

// Function to update effects and modulations
function updateEffects() {
    const now = Tone.now();
    panner.pan.linearRampToValueAtTime(Math.random() * 2 - 1, now + 20);
    masterFilter.frequency.linearRampToValueAtTime(Math.random() * 1500 + 80, now + 30);
    bassDrone.spread = Math.random() * 80 + 20;
    
    bassLFO.frequency.linearRampToValueAtTime(Math.random() * 0.05, now + 20);
    padLFO.frequency.linearRampToValueAtTime(Math.random() * 0.08, now + 20);
    filterLFO.frequency.linearRampToValueAtTime(Math.random() * 0.03, now + 20);
    textureFilter.baseFrequency = Math.random() * 200 + 50;

    const bassVolume = Math.max(Math.random() * 30 - 30, -20);
    bassDrone.volume.linearRampToValueAtTime(bassVolume, now + Math.random() * 20 + 10);

    const textureVolume = Math.random() * 20 - 80;
    texture.volume.linearRampToValueAtTime(textureVolume, now + Math.random() * 60 + 30);

    console.log("Effects updated");
}

// Schedule random events with varying intervals
function scheduleEvents() {
    const nextPadTime = Math.random() * 10 + 10;
    const nextAccentTime = Math.random() * 5 + 5;
    const nextEffectTime = Math.random() * 10 + 10;

    const now = Tone.now();

    Tone.Transport.scheduleOnce(() => {
        playPadChord();
    }, now + nextPadTime);

    Tone.Transport.scheduleOnce(() => {
        playAccent();
    }, now + nextAccentTime);

    Tone.Transport.scheduleOnce(() => {
        updateEffects();
    }, now + nextEffectTime);

    Tone.Transport.scheduleOnce(() => {
        scheduleEvents();
    }, now + Math.max(nextPadTime, nextAccentTime, nextEffectTime) + 1);
}

function playAmbientSound() {
    bassDrone.start();
    playPadChord();
    updateEffects();

    Tone.Transport.start();
    scheduleEvents();
    toggleBassDrone();
    console.log("Ambient Sound is playing");
}

function stopAmbientSound() {
    bassDrone.stop();
    Tone.Transport.stop();
    console.log("Ambient Sound stopped");
}

window.playAmbientSound = playAmbientSound;
window.stopAmbientSound = stopAmbientSound;
