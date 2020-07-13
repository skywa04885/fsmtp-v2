"use strict";
(function () {
    var _a;
    var loaderOverlay = document.querySelector('#loadingOverlay');
    var fannstAccountMenu = document.querySelector('#fannstAccountMenu');
    var fannstAccountIcon = document.querySelector('#fannstAccountIcon');
    // Adds the account menu toggler
    fannstAccountIcon.addEventListener('click', function (e) {
        fannstAccountMenu.hidden = !fannstAccountMenu.hidden;
    });
    // Selects all the anchors and
    // - adds the event listeners
    (_a = document.querySelectorAll('a')) === null || _a === void 0 ? void 0 : _a.forEach(function (anchor) {
        anchor.addEventListener('click', function (e) {
            e.preventDefault();
            // Shows the loader and waits an short amount of time
            loaderOverlay.hidden = false;
            setTimeout(function () {
                var a = (e.target);
                window.location.href = a.href;
            }, 180);
        });
    });
})();
