// components/health-manager.js
/**
 * @component health-manager
 * @description Manages health of cannons
 */
AFRAME.registerComponent('health-manager', {
    init: function() {
        this.blueHp = document.querySelector('#blueHp');
        this.greenHp = document.querySelector('#greenHp');
        this.blueHpAmt = 3;
        this.greenHpAmt = 3;

        this.el.addEventListener('collision-detected', (event) => {
            this.handleHp(event.detail.isGreenTurn);
        });
    },

    handleHp: function(isGreenTurn) {
        const targetHp = isGreenTurn ? this.blueHp : this.greenHp;
        const targetHpAmt = isGreenTurn ? this.blueHpAmt : this.greenHpAmt;

        targetHp.setAttribute('value', 'Health: ' + (targetHpAmt - 1));
        if (isGreenTurn) {
            this.blueHpAmt--;
        } else {
            this.greenHpAmt--;
        }

        if (this.blueHpAmt <= 0 || this.greenHpAmt <= 0) {
            console.log("Game Over!");
            this.resetHp();
        }
    },

    resetHp: function() {
        // Show win/lose messages
        if (this.blueHpAmt <= 0) {
            this.blueHp.setAttribute('value', "You Lose");
            this.greenHp.setAttribute('value', "You Win");
        } else {
            this.greenHp.setAttribute('value', "You Lose");
            this.blueHp.setAttribute('value', "You Win");
        }
        
        this.el.emit('stop-turn');

        // Wait 5 seconds before resetting the health
        setTimeout(() => {
            this.blueHpAmt = 3;
            this.greenHpAmt = 3;
            this.greenHp.setAttribute('value', 'Health: ' + this.greenHpAmt);
            this.blueHp.setAttribute('value', 'Health: ' + this.blueHpAmt);
            this.el.emit('switch-turn');
        }, 5000);
    }
});