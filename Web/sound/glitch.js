// Glitch sound setup
const glitchSynth = new Tone.NoiseSynth().toDestination();

function playGlitchSound() {
    glitchSynth.triggerAttackRelease("8n");
    setTimeout(() => glitchSynth.triggerAttackRelease("8n"), 1000); // Example glitchy effect
}

window.playGlitchSound = playGlitchSound;
