// components/countdown-manager.js
/**
 * @component countdown-manager
 * @description Manages turn-based countdown system between two cannons
 */
AFRAME.registerComponent('countdown-manager', {
    init: function() {
      this.isGreenTurn = true; // Start with green
      this.countdown = 5;
      this.isRunning = false;
      
      // Get references to both text elements
      this.greenText = document.querySelector('#greenTimer');
      this.blueText = document.querySelector('#blueTimer');

      // Get cannonball element
      this.cannonball = document.querySelector('#cannonball');

      // Get cannons
      this.greenCannon = document.querySelector('#greenCannon');
      this.blueCannon = document.querySelector('#blueCannon');

      // Create two position vectors
      this.greenCannonPos = new THREE.Vector3();
      this.blueCannonPos = new THREE.Vector3();

      // Cannon tips
      // Get cannons
      this.greenTip = document.querySelector('#greenCannon');
      this.blueTip = document.querySelector('#blueCannon');
      this.greenTipPos = new THREE.Vector3();
      this.blueTipPos = new THREE.Vector3();

      // Initialize animation state
      this.animationActive = false;
      
      // Start when either marker is found
      this.el.sceneEl.addEventListener('markerFound', () => {
        if (!this.isRunning) {
          this.startCountdown();
        }
      });
    },
  
    startCountdown: function() {
      this.isRunning = true;
      this.countdown = 5;
      
      const timer = setInterval(() => {
        // Update current player's text
        if (this.isGreenTurn) {
          this.greenText.setAttribute('value', this.countdown.toString());
          this.blueText.setAttribute('value', '');
        } else {
          this.blueText.setAttribute('value', this.countdown.toString());
          this.greenText.setAttribute('value', '');
        }
        
        this.countdown--;
        
        if (this.countdown < 0) {
          // Shoot cannonball
          if (!this.animationActive) {
            this.animationActive = true;
            this.shootCannonBall();
          }
          
          // Switch turns
          this.isGreenTurn = !this.isGreenTurn;
          this.countdown = 5;
          
          clearInterval(timer);
          setTimeout(() => {
            this.startCountdown();
          }, 500);
        }
      }, 1000);
    },

    shootCannonBall: function() {
      console.log("Shoot cannon")
      // Get world positions of both markers
      this.greenTip.object3D.getWorldPosition(this.greenTipPos);
      this.blueTip.object3D.getWorldPosition(this.blueTipPos);

      // TODO:: Render at the dog fucking crap bitch ass cannon tip

      // Make cannonball visible
      this.cannonball.setAttribute('visible', true);
      // Shoot right if green, left if blue
      const startPos = this.isGreenTurn ? this.greenTipPos : this.blueTipPos;
      const direction =  this.isGreenTurn ? 10 : -10;
      
      // Set up animation for the spear
      this.cannonball.setAttribute('animation', {
        property: 'position',
        from: `${startPos.x} ${startPos.y} ${startPos.z}`,
        to: `${startPos.x + direction} ${startPos.y} ${startPos.z}`,
        dur: 2000,
        dir: 'linear',
      });
      
      // TODO:: Add logic to stop make ball inivisble once it hits the other cannon

      setTimeout(() => {
        this.animationActive = false;
        this.cannonball.setAttribute('visible', false);
        this.cannonball.removeAttribute('animation');
      }, 4000);
    }
  });