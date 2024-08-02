// Pressed sound setup
const pressedSynth = new Tone.FMSynth().toDestination();
const bassSynth = new Tone.MonoSynth({
    oscillator: { type: "sine" },
    envelope: { attack: 0, decay: 0, sustain: 1, release: 10 }
}).toDestination();
const bassOscillator = new Tone.Oscillator("C2", "sine").toDestination();
// const notesForMultiplier = [["C2", "D4"], ["G2", "F4"], ["D3", "E4"], ["A3", "B4"]];
const notesForMultiplier = [["C2", "D4"], ["F2", "G4"], ["Bb3", "C5"], ["Eb3", "F5"]];

function playPressedSound(speedMultiplier) {
    bassSynth.triggerAttackRelease(notesForMultiplier[speedMultiplier][0], "4n");
    pressedSynth.triggerAttackRelease(notesForMultiplier[speedMultiplier][1], "4n");
    // bassOscillator.frequency.value = 220 * speedMultiplier;  // 220 Hz is A2
    // bassOscillator.start();
    // setTimeout(() => bassOscillator.stop(), 500);  // Plays for 500 ms
}

window.playPressedSound = playPressedSound;
