// components/sound-control.js
/**
 * @component sound-control
 * @description Manages sound control
 */
AFRAME.registerComponent('sound-control', {
    init: function () {
        // Get the background music element
        const bgSound = document.querySelector('#bgSound');

        // press M to mute
        window.addEventListener('keydown', (event) => {
            if (event.key === '1') {
                bgSound.components.sound.stopSound();
            }
            if (event.key === '2') {
                bgSound.components.sound.pauseSound();
            }
            if (event.key === '3') {{
                bgSound.components.sound.playSound();
            }}
        });
    }
})