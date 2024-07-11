let bassOscillator, buttonLfo, buttonVolume, buttonDistortion, buttonReverb, buttonLowpass, buttonCompressor, activationSynth, activationNoiseSynth;

function initializeButtonSounds() {
    buttonVolume = new Tone.Volume(0.6).toDestination(); // Increase the volume level for the bass sound
    buttonDistortion = new Tone.Distortion(0.4).connect(buttonVolume);
    buttonReverb = new Tone.Reverb({
        decay: 5,
        wet: 0.6
    }).connect(buttonDistortion);
    buttonLowpass = new Tone.Filter({
        type: "lowpass",
        frequency: 200,
        rolloff: -24,
        Q: 1
    }).connect(buttonReverb);
    buttonCompressor = new Tone.Compressor({
        threshold: -30,
        ratio: 6
    }).connect(buttonLowpass);

    bassOscillator = new Tone.Oscillator({
        frequency: "A0", // Lower frequency for deeper bass
        type: "sine"
    }).connect(buttonCompressor);

    buttonLfo = new Tone.LFO({
        frequency: 0.5, // Initial LFO frequency
        min: 27.5,
        max: 30,
        type: "sine"
    }).connect(bassOscillator.frequency);

    // New activation sound setup
    activationSynth = new Tone.Synth({
        oscillator: {
            type: "triangle",
            frequency: "C2"
        },
        envelope: {
            attack: 0.05,
            decay: 0.2,
            sustain: 0.5,
            release: 1
        }
    });

    activationNoiseSynth = new Tone.NoiseSynth({
        noise: {
            type: "brown"
        },
        envelope: {
            attack: 0.1,
            decay: 0.4,
            sustain: 0.3,
            release: 0.5
        }
    });

    const activationReverb = new Tone.Reverb({
        decay: 2,
        wet: 0.5
    }).toDestination();

    const activationFilter = new Tone.Filter({
        type: "lowpass",
        frequency: 800,
        Q: 1
    }).toDestination();

    activationSynth.chain(activationFilter, activationReverb);
    activationNoiseSynth.chain(activationFilter, activationReverb);
}

function handleButtonPress(buttonCount) {
    if (buttonCount === 0) {
        stopTone();
        fadeInAmbientSound(); // Fade in ambient sound when no buttons are pressed
    } else {
        fadeOutAmbientSound(); // Fade out ambient sound when a button is pressed
        if (bassOscillator.state !== 'started') {
            bassOscillator.start();
            buttonLfo.start();
        }
        buttonVolume.volume.rampTo(0.6, 1); // Increased volume for the bass sound
        adjustLfoFrequency(buttonCount);
        playActivationSound(); // Play activation sound on button press
    }
}

function adjustLfoFrequency(buttonCount) {
    if (buttonCount === 1) {
        buttonLfo.frequency.rampTo(1, 1); // Increase LFO frequency to 1 Hz over 1 second
    } else if (buttonCount === 2) {
        buttonLfo.frequency.rampTo(2, 1); // Increase LFO frequency to 2 Hz over 1 second
    }
}

function stopTone() {
    buttonLfo.frequency.rampTo(0.5, 1); // Reset LFO frequency to 0.5 Hz over 1 second
    buttonVolume.volume.rampTo(-Infinity, 1); // Gradually reduce volume to silence over 1 second
}

function playActivationSound() {
    const now = Tone.now();
    activationSynth.triggerAttackRelease("C2", "1n", now);
    activationNoiseSynth.triggerAttackRelease("2n", now);
}
