let ambientDrone, ambientDrone2, ambientReverb, ambientDelay, noiseFilter, noisePanner, noiseGain, heartbeat, playerGain, ambientGain, currentPlayer;
let audioFiles = ["sounds/High.m4a", "sounds/Knows.m4a", "Whaley.m4a"];

function initializeAmbientSound() {
    ambientGain = new Tone.Gain(0.3).toDestination(); // Lower initial volume for ambient sounds

    // Ambient drone setup
    ambientDrone = new Tone.AMOscillator({
        harmonicity: Math.random(),
        modulationType: "sine",
        frequency: "C2",
        type: "sine",
        modulationIndex: Math.random() * 5,
        detune: Math.random() * 100 - 50
    }).start();

    // Additional ambient drone for complexity
    ambientDrone2 = new Tone.FMOscillator({
        harmonicity: Math.random() * 2,
        modulationType: "sine",
        frequency: "G1",
        type: "sine",
        modulationIndex: Math.random() * 5,
        detune: Math.random() * 100 - 50
    }).start();

    ambientReverb = new Tone.Reverb({
        decay: Math.random() * 10 + 5,
        preDelay: Math.random() * 0.5,
        wet: 0.7
    }).connect(ambientGain);

    ambientDelay = new Tone.FeedbackDelay({
        delayTime: "4n", // Set a valid delay time
        feedback: 0.6,
        wet: 0.5
    }).connect(ambientGain);

    const dronePanner = new Tone.Panner(0).connect(ambientGain);
    const dronePanner2 = new Tone.Panner(0).connect(ambientGain);

    const lfoPanner = new Tone.LFO({
        frequency: Math.random() * 0.1 + 0.05,
        min: -1,
        max: 1,
        type: "sine"
    }).connect(dronePanner.pan).start();

    const lfoPanner2 = new Tone.LFO({
        frequency: Math.random() * 0.1 + 0.02,
        min: -1,
        max: 1,
        type: "sine"
    }).connect(dronePanner2.pan).start();

    const lfoFreq = new Tone.LFO({
        frequency: Math.random() * 0.1 + 0.01,
        min: Tone.Frequency("C1").toFrequency(),
        max: Tone.Frequency("C3").toFrequency(),
        type: "sine"
    }).connect(ambientDrone.frequency).start();

    const lfoFreq2 = new Tone.LFO({
        frequency: Math.random() * 0.05 + 0.01,
        min: Tone.Frequency("G1").toFrequency(),
        max: Tone.Frequency("G3").toFrequency(),
        type: "sine"
    }).connect(ambientDrone2.frequency).start();

    ambientDrone.chain(ambientReverb, ambientDelay, dronePanner, ambientGain);
    ambientDrone2.chain(ambientReverb, ambientDelay, dronePanner2, ambientGain);

    // Heartbeat setup
    heartbeat = new Tone.MembraneSynth({
        oscillator: {
            type: "sine"
        },
        envelope: {
            attack: 0.02,
            decay: 0.8,
            sustain: 0,
            release: 0.8
        }
    }).connect(ambientGain);

    function playHeartbeat() {
        heartbeat.triggerAttackRelease("C1", "8n", Tone.now());
        setTimeout(playHeartbeat, Math.random() * 3000 + 2000); // Play every 2-5 seconds randomly
    }
    playHeartbeat();

    playerGain = new Tone.Gain(0.3).toDestination(); // Lower initial volume for player

    playRandomAudioFile();
}

function playRandomAudioFile() {
    if (currentPlayer) {
        currentPlayer.stop();
        currentPlayer.dispose();
    }

    const randomFile = audioFiles[Math.floor(Math.random() * audioFiles.length)];
    console.log(`Loading audio file: ${randomFile}`);
    
    currentPlayer = new Tone.Player({
        url: randomFile,
        loop: false,
        autostart: true,
        onload: () => {
            console.log(`Audio file loaded: ${randomFile}`);
            applyAudioEffects(currentPlayer);
            console.log(`Playing audio file: ${randomFile}`);
        },
        onerror: (error) => {
            console.error(`Error loading audio file: ${randomFile}`, error);
        }
    }).connect(playerGain);

    const nextPlayTime = Math.random() * (3 - 0.5) + 0.5; // Random interval between 30 seconds and 3 minutes
    setTimeout(playRandomAudioFile, nextPlayTime * 60000);
}

function applyAudioEffects(player) {
    const reverb = new Tone.Reverb({
        decay: Math.random() * 5 + 2, // Random reverb decay between 2-7 seconds
        wet: Math.random() * 0.5 + 0.5 // Random wet level between 0.5-1
    }).toDestination();

    const panner = new Tone.Panner(Math.random() * 2 - 1).toDestination(); // Random pan between -1 (left) and 1 (right)

    const delay = new Tone.FeedbackDelay({
        delayTime: "8n",
        feedback: Math.random() * 0.2 + 0.3, // Random feedback between 0.3-0.5
        wet: Math.random() * 0.2 + 0.5 // Random wet level between 0.5-0.7
    }).toDestination();

    const filter = new Tone.Filter({
        type: 'highpass',
        frequency: Math.random() * 200 + 500, // Random highpass filter frequency between 500-700
        rolloff: -24,
        Q: 1
    }).toDestination();

    player.chain(reverb, panner, delay, filter);
}

function fadeOutAmbientSound() {
    if (ambientGain) {
        ambientGain.gain.rampTo(0, 1); // Fade out ambient sound over 1 second
    } else {
        console.error("ambientGain is not initialized.");
    }

    if (playerGain) {
        playerGain.gain.rampTo(0, 1); // Fade out player sound over 1 second
    } else {
        console.error("playerGain is not initialized.");
    }

    if (currentPlayer) {
        currentPlayer.stop();
    }
}

function fadeInAmbientSound() {
    if (ambientGain) {
        ambientGain.gain.rampTo(0.3, 1); // Fade in ambient sound over 1 second, match initial gain level
    } else {
        console.error("ambientGain is not initialized.");
    }

    if (playerGain) {
        playerGain.gain.rampTo(0.3, 1); // Fade in player sound over 1 second, match initial gain level
    } else {
        console.error("playerGain is not initialized.");
    }

    if (currentPlayer && !currentPlayer.state.includes('started')) {
        currentPlayer.start();
    }
}
