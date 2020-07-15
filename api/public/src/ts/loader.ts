(() => {
  const loaderOverlay: HTMLDivElement = <HTMLDivElement>document.querySelector('#loadingOverlay');
  const fannstAccountMenu: HTMLDivElement = <HTMLDivElement>document.querySelector('#fannstAccountMenu');
  const fannstAccountIcon: HTMLButtonElement = <HTMLButtonElement>document.querySelector('#fannstAccountIcon');

  // Adds the account menu toggler
  fannstAccountIcon.addEventListener('click', (e: Event) => {
    fannstAccountMenu.hidden = !fannstAccountMenu.hidden;
  });

  // Selects all the anchors and
  // - adds the event listeners
  document.querySelectorAll('a')?.forEach((anchor: HTMLAnchorElement) => {
    anchor.addEventListener('click', (e: Event) => {
      e.preventDefault();

      // Shows the loader and waits an short amount of time
      loaderOverlay.hidden = false;
      setTimeout(() => {
        let a: HTMLAnchorElement = <HTMLAnchorElement>(e.target);
        window.location.href = a.href;
      }, 5);
    });
  });
})();