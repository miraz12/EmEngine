var canvas = document.getElementById("your-canvas-element");
var inputManager = Module.cwrap('HandleTouchMove', null, ['number', 'number']);

canvas.addEventListener("touchmove", handleTouchMove, false);

function handleTouchMove(event) {
    var touch = event.touches[0];
    var touchCurrentPosition = {
        x: touch.clientX,
        y: touch.clientY
    };

    // Calculate the movement distance
    var deltaX = touchCurrentPosition.x - touchStartPosition.x;
    var deltaY = touchCurrentPosition.y - touchStartPosition.y;

    // Call the C++ function to handle touch movement
    inputManager(deltaX, deltaY);
}
