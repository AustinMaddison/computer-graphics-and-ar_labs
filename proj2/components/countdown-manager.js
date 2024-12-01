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
      
      // Shoot right if green, left if blue
      const startPos = this.isGreenTurn ? this.greenTipPos : this.blueTipPos;
      const targetCannon = this.isGreenTurn ? this.blueCannon : this.greenCannon;
      const direction =  this.isGreenTurn ? 10 : -10;
      // Reset cannonball position to the starting position
      this.cannonball.object3D.position.set(startPos.x, startPos.y, startPos.z);
      // Make cannonball visible
      this.cannonball.setAttribute('visible', true);

      // Set up animation for the spear
      this.cannonball.setAttribute('animation', {
        property: 'position',
        from: `${startPos.x} ${startPos.y} ${startPos.z}`,
        to: `${startPos.x + direction} ${startPos.y} ${startPos.z}`,
        dur: 4000,
        dir: 'linear',
      });

      // Start collision check
      this.checkCollision(targetCannon);

      setTimeout(() => {
        this.animationActive = false;
        this.cannonball.setAttribute('visible', false);
        this.cannonball.removeAttribute('animation');
      }, 4000);
    },

    checkCollision: function(targetCannon) {
      // Check for collision every 50ms
      const collisionInterval = setInterval(() => {
        // Get positions of cannonball and target cannon
        const cannonballPos = new THREE.Vector3();
        const targetCannonPos = new THREE.Vector3();

        this.cannonball.object3D.getWorldPosition(cannonballPos);
        targetCannon.object3D.getWorldPosition(targetCannonPos);
        
        // Calculate distance between cannonball and target cannon
        const distance = cannonballPos.distanceTo(targetCannonPos);

        // Radius according to how we rnder the cannons
        const hitboxRadius = 0.8;

        if (distance < hitboxRadius) {
          console.log("Boom!");
          
          // Make cannonball invisible and stop animation
          this.animationActive = false;
          this.cannonball.setAttribute('visible', false);
          this.cannonball.removeAttribute('animation');
        }
        // Clear interval once the animation is finished
        if (!this.animationActive) {
          clearInterval(collisionInterval);
        }
      }, 50);
    }
  });