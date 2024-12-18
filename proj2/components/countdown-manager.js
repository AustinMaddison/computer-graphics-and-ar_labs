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
      
      // countdown sound
      this.countdownSound = document.querySelector('#countdown');
      
      // references to hp
      this.blueHp = document.querySelector('#blueHp');
      this.greenHp = document.querySelector('#greenHp');
      this.blueHpAmt = 3
      this.greenHpAmt = 3

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

      this.el.addEventListener('switch-turn', () => {
        this.switchTurn();
      });

      this.healthEntity = document.querySelector('[health-manager]');
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
        
        if (this.countdown === 5) {
            // Play countdown sound effect
          this.countdownSound.components.sound.playSound();
        }

        this.countdown--;

        if (this.countdown < 0) {
          this.countdownSound.components.sound.stopSound();
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
      const isGreenTurn = this.isGreenTurn;
      const direction =  this.isGreenTurn ? 1 : -1;

      // Reset cannonball position to the starting position
      this.cannonball.object3D.position.set(startPos.x, startPos.y, startPos.z);
      // Make cannonball visible
      this.cannonball.setAttribute('visible', true);

      // Set initial velocity components
      const initialVelocity = 5; // Modify this value to adjust the cannonball's speed
      const gravity = 9.8; // Gravity acceleration
      const angle = Math.PI / 4; // 45 degrees for a standard parabolic trajectory
      // Split initial velocity into horizontal and vertical components
      const v_x = initialVelocity * Math.cos(angle);
      const v_y = initialVelocity * Math.sin(angle);

      // Play shoot cannon sound effect
      const shootSound = document.querySelector('#cannonSound')
      shootSound.components.sound.playSound();

      // Start the animation loop to simulate parabolic motion
      let startTime = null;
      const duration = 5000; // Duration of the entire flight in milliseconds
      const totalTime = duration / 1000; // Convert to seconds
      let elapsedTime = 0;

      const animate = (timestamp) => {
        if (!startTime) startTime = timestamp;
        elapsedTime = (timestamp - startTime) / 1000; // Elapsed time in seconds

        // Calculate new position based on parabolic equations
        const deltaX = v_x * elapsedTime; // Horizontal displacement
        const deltaY = v_y * elapsedTime - (0.5 * gravity * Math.pow(elapsedTime, 2)); // Vertical displacement

        // Update the cannonball's position
        this.cannonball.object3D.position.set(startPos.x + deltaX * direction, startPos.y + deltaY, startPos.z);

        // stop when overtime
        if (elapsedTime >= totalTime) {
          console.log("timeout");
          this.cannonball.setAttribute('visible', false); // Hide the cannonball after impact
          this.animationActive = false; // Reset animation state
          return;
        }

        // Continue the animation
        if (this.animationActive) {
          requestAnimationFrame(animate); // Continue the animation loop
        }
      };

      // Start the animation loop
      this.animationActive = true;
      requestAnimationFrame(animate);

      // Check for collisions during the flight
      this.checkCollision(isGreenTurn);

      // Just incase something byebye and breaks
      setTimeout(() => {
        this.animationActive = false;
        this.cannonball.setAttribute('visible', false);
        this.cannonball.removeAttribute('animation');
      }, 10000);
    },

    checkCollision: function(isGreenTurn) {

      targetCannon = isGreenTurn ? this.blueCannon : this.greenCannon;

      let collisionDetected = false; // Add a flag to prevent multiple deductions

      // Check for collision every 50ms
      const collisionInterval = setInterval(() => {
        if (collisionDetected || !this.animationActive) {
          clearInterval(collisionInterval);
          return; // Exit if a collision has been handled or animation is inactive
        }
        // Get positions of cannonball and target cannon
        const cannonballPos = new THREE.Vector3();
        const targetCannonPos = new THREE.Vector3();

        this.cannonball.object3D.getWorldPosition(cannonballPos);
        targetCannon.object3D.getWorldPosition(targetCannonPos);
        
        // Calculate distance between cannonball and target cannon 
        // const distance = cannonballPos.distanceTo(targetCannonPos); -> Doesn't work cuz z axis is trolling me

        // Calculate the distance between cannonball and target cannon, ignoring the z-axis
        const dx = cannonballPos.x - targetCannonPos.x; 
        const dy = cannonballPos.y - targetCannonPos.y; 

        // Calculate the 2D distance
        const distance = Math.sqrt(dx * dx + dy * dy);

        // Radius according to how we rnder the cannons
        const hitboxRadius = 0.8;

        if (distance < hitboxRadius) {
          collisionDetected = true; // Set the flag to true
          // Show collision feedback
          this.showCollisionFeedback(isGreenTurn);
          
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
        if (!this.animationActive || collisionDetected) {
          clearInterval(collisionInterval);
          // Switch turn after collision
          this.switchTurn();
        }
      }, 50);
    },

    showCollisionFeedback: function(isGreenTurn) {
      // calls this.handleHp(isGreenTurn)
      this.healthEntity.emit('collision-detected', { isGreenTurn });

      const targetCannon = isGreenTurn ? this.blueCannon : this.greenCannon;
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
      this.isRunning = true;
      this.isGreenTurn = !this.isGreenTurn;
      this.startCountdown();
    },
  });