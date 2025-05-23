import { defineConfig } from 'tsup'

export default defineConfig({
  entry: ['src/index.ts'],
  splitting: false,
  sourcemap: true,
  clean: true,
  dts: true,
  format: 'esm',
  outDir: 'dist',
  target: 'esnext',
  platform: 'browser',
  tsconfig: 'tsconfig.json',
})