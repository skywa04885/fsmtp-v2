(() => {
  const registerForm: HTMLFormElement = <HTMLFormElement>(document.querySelector('#registerForm'));
  const loaderOverlay: HTMLDivElement = <HTMLDivElement>document.querySelector('#loadingOverlay');
  const errorMessage: HTMLDivElement = <HTMLDivElement>document.querySelector('#errorMessage');

  const fullnameInput: HTMLInputElement = <HTMLInputElement>document.querySelector('#a_full_name');
  const usernameInput: HTMLInputElement = <HTMLInputElement>document.querySelector('#a_username');
  const domainInput: HTMLInputElement = <HTMLInputElement>document.querySelector('#a_domain');
  const passwordInput: HTMLInputElement = <HTMLInputElement>document.querySelector('#a_password');
  const passwordConfirmInput: HTMLInputElement = <HTMLInputElement>document.querySelector('#a_password_confirm');

  passwordConfirmInput.onchange = (): void => {
    if (passwordInput.value !== passwordConfirmInput.value)
      passwordConfirmInput.setCustomValidity('Passwords do not match');
    else
      passwordConfirmInput.setCustomValidity('');
  };

  // Adds the event listener
  registerForm.addEventListener('submit', (e: Event) => {
    e.preventDefault();

    // Checks if the confirm password does match
    // - the password

    // Creates the request and sends the
    // - data to the server
    loaderOverlay.hidden = false;
    let xhr: XMLHttpRequest = new XMLHttpRequest();
    xhr.open('POST', '/accounts/register');
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.onreadystatechange = () => {
      if (xhr.readyState === 4)
      {
        const resp: any = JSON.parse(xhr.responseText); 
        loaderOverlay.hidden = true;
        switch (xhr.status)
        {
          case 200: case 500:
          {
            if (!resp.status)
            {
              errorMessage.hidden = false;
              errorMessage.innerText = resp.message;
            } else
            {
              errorMessage.hidden = true;

              // Redirects
              loaderOverlay.hidden = false;
              setTimeout(() => {
                window.location.href = '/accounts/login';
              }, 280);
            }
            break;
          }
        }
      }
    };
    setTimeout(() => {
      xhr.send(JSON.stringify({
        a_full_name: fullnameInput.value,
        a_username: usernameInput.value,
        a_domain: domainInput.value !== '' ? domainInput.value : 'fannst.nl',
        a_password: passwordInput.value
      }));
    }, 250);
  });
})();