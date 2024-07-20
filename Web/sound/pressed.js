// Pressed sound setup
const pressedSynth = new Tone.Synth().toDestination();
const bassOscillator = new Tone.Oscillator("C2", "sine").toDestination();

function playPressedSound(speedMultiplier) {
    pressedSynth.triggerAttackRelease("C4", "8n");
    bassOscillator.frequency.value = 220 * speedMultiplier;  // 220 Hz is A2
    bassOscillator.start();
    setTimeout(() => bassOscillator.stop(), 500);  // Plays for 500 ms
}

window.playPressedSound = playPressedSound;
