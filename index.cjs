#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

const targetPath = path.resolve(process.argv[2] || '.');

// Required files
const files = [
  'libffmpeg_265.js',
  'libffmpeg_265.wasm'
];

// Create target directory if it doesn't exist
if (!fs.existsSync(targetPath)) {
  fs.mkdirSync(targetPath, { recursive: true });
}

// Copy files
files.forEach(file => {
  const sourcePath = path.join(__dirname, file);
  const targetFilePath = path.join(targetPath, file);

  try {
    fs.copyFileSync(sourcePath, targetFilePath);
    console.log(`Copied ${file} to ${targetPath}`);
  } catch (error) {
    console.error(`Error copying ${file}:`, error.message);
    process.exit(1);
  }
});

console.log('\nInstallation complete!');
console.log(`WASM and JS files have been copied to ${targetPath}`);
