const path = require('path');
const fs = require('fs');
const { spawn } = require('child_process');

const ROOT = path.join(__dirname, '../');
const IGNORE_DIRS = ['build', '.vscode', '.idea', 'subprojects'].map(dir => path.join(ROOT, dir));

let running = false;
let proc = undefined;
const run = () => {
  if (running) {
    proc.kill('SIGKILL');
    while (!proc.killed) continue;
    running = false;
  }

  proc = spawn('./fsmtp', []);
  running = true;

  proc.stdout.on('data', data => {
    process.stdout.write(data.toString());
  });

  proc.stderr.on('data', data => {
    process.stderr.write(data.toString());
  });
};

let rebuilding = false;
const rebuild = (event, filename) => {
  if (rebuilding) return;
  else rebuilding = true;
  console.log(`File ${filename} changed, rebuilding ...`);

  const ninja = spawn('ninja', []);

  ninja.stderr.on('data', data => {
    process.stderr.write('\033[31m' + data.toString() + '\033[0m');
  });

  ninja.stdout.on('data', data => {
    process.stdout.write('\033[35m' + data.toString() + '\033[0m');
  });

  ninja.on('exit', code => {
    rebuilding = false;
    if (code === 0) {
      run();
    } else console.error(`Ninja failed: ${code}`);
  });
};

const addWatchers = (dir) => {
  console.info(`Adding watchers for dir: ${dir}`);

  // Reads the sub directory's from the dir
  //  so we can check if they contain more dirs we can
  //  listen to for changes

  fs.readdir(dir, (err, files) => {
    if (err) {
      return console.error(err);
    } else fs.watch(dir, {}, rebuild);

    // Loops over the sub directory's and starts
    //  adding them to the watchers as well, if the
    //  dir is hidden, we ignore it
    files.forEach((subdir) => {
      if (subdir[0] === '.') return;

      let subdirJoined = path.join(dir, subdir);
      if (fs.statSync(subdirJoined).isDirectory()) {
        if (IGNORE_DIRS.indexOf(subdirJoined) !== -1) return;

        addWatchers(subdirJoined);
      }
    });
  });
}

addWatchers(ROOT);
rebuild();