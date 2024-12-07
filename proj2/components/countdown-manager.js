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
      this.greenTip = document.querySelector('#greenCannonTip');
      this.blueTip = document.querySelector('#blueCannonTip');
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
          
          clearInterval(timer);
        }
      }, 1000);
    },

    shootCannonBall: function() {
      console.log("Shoot cannon")
      // Get world positions of both markers
      this.greenTip.object3D.getWorldPosition(this.greenTipPos);
      this.blueTip.object3D.getWorldPosition(this.blueTipPos);
      
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
        dur: 10000,
        dir: 'linear',
      });

      // Start collision check
      this.checkCollision(targetCannon);

      // Just incase something byebye and breaks
      setTimeout(() => {
        this.animationActive = false;
        this.cannonball.setAttribute('visible', false);
        this.cannonball.removeAttribute('animation');
      }, 10000);
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

          // Show collision feedback
          this.showCollisionFeedback(targetCannon);
          
          // Make cannonball invisible and stop animation
          this.cannonball.setAttribute('visible', false);
          this.cannonball.removeAttribute('animation');
        }

        const camera = this.el.sceneEl.camera; // Get the camera
        // Project the position into the camera's NDC
        cannonballPos.project(camera);
        // Check if the position is out of bounds
        if (
            cannonballPos.x < -1 || cannonballPos.x > 1 || // X out of bounds
            cannonballPos.y < -1 || cannonballPos.y > 1   // Y out of bounds
        ) {
          console.log("Cannonball is out of bounds!");

          // Stop animation and make the cannonball invisible
          this.cannonball.setAttribute('visible', false);
          this.cannonball.removeAttribute('animation');

          // Reset animation state
          this.animationActive = false;
        }
        
        // Clear interval once the animation is finished
        if (!this.animationActive) {
          clearInterval(collisionInterval);
          // Switch turn after collision
          this.switchTurn();
        }
      }, 50);
    },

    showCollisionFeedback: function(targetCannon) {
      console.log("oH NOoooo ded")
      const redOutline = document.createElement('a-plane');
      redOutline.setAttribute('color', 'red');
      redOutline.setAttribute('width', '1.1'); // Adjust width (10% larger)
      redOutline.setAttribute('height', '1.1'); // Adjust height (10% larger)
      redOutline.setAttribute('position', '0 0 0'); // Centered on the cannon
      redOutline.setAttribute('rotation', '-90 0 0'); // Align horizontally

      // Attach the thing to the target cannon
      targetCannon.appendChild(redOutline);

      // Remove the thing after 1 second
      setTimeout(() => {
        this.animationActive = false;
        targetCannon.removeChild(redOutline);
      }, 1000);
    },

    switchTurn: function () {
      console.log("Switching turn...");
      this.isGreenTurn = !this.isGreenTurn;
      this.startCountdown();
    },
  });

// TODO:: Position at cannon tip
// TODO:: Make the ball abit faster
// TODO:: Parabolic Motion for ball