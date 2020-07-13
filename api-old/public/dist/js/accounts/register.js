"use strict";
(function () {
    var registerForm = (document.querySelector('#registerForm'));
    var loaderOverlay = document.querySelector('#loadingOverlay');
    var errorMessage = document.querySelector('#errorMessage');
    var fullnameInput = document.querySelector('#a_full_name');
    var usernameInput = document.querySelector('#a_username');
    var domainInput = document.querySelector('#a_domain');
    var passwordInput = document.querySelector('#a_password');
    var passwordConfirmInput = document.querySelector('#a_password_confirm');
    passwordConfirmInput.onchange = function () {
        if (passwordInput.value !== passwordConfirmInput.value)
            passwordConfirmInput.setCustomValidity('Passwords do not match');
        else
            passwordConfirmInput.setCustomValidity('');
    };
    // Adds the event listener
    registerForm.addEventListener('submit', function (e) {
        e.preventDefault();
        // Checks if the confirm password does match
        // - the password
        // Creates the request and sends the
        // - data to the server
        loaderOverlay.hidden = false;
        var xhr = new XMLHttpRequest();
        xhr.open('POST', '/accounts/register');
        xhr.setRequestHeader('Content-Type', 'application/json');
        xhr.onreadystatechange = function () {
            if (xhr.readyState === 4) {
                var resp = JSON.parse(xhr.responseText);
                loaderOverlay.hidden = true;
                switch (xhr.status) {
                    case 200:
                    case 500:
                        {
                            if (!resp.status) {
                                errorMessage.hidden = false;
                                errorMessage.innerText = resp.message;
                            }
                            else {
                                errorMessage.hidden = true;
                                // Redirects
                                loaderOverlay.hidden = false;
                                setTimeout(function () {
                                    window.location.href = '/accounts/login';
                                }, 280);
                            }
                            break;
                        }
                }
            }
        };
        setTimeout(function () {
            xhr.send(JSON.stringify({
                a_full_name: fullnameInput.value,
                a_username: usernameInput.value,
                a_domain: domainInput.value !== '' ? domainInput.value : 'fannst.nl',
                a_password: passwordInput.value
            }));
        }, 250);
    });
})();
