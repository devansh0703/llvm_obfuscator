# Electron GUI for Adaptive Obfuscator

Cross-platform desktop GUI for the Adaptive LLVM Obfuscator.

## Building

### Prerequisites
- Node.js 18+
- npm or yarn

### Installation

```bash
cd gui
npm install
```

### Development

```bash
npm run dev
```

### Build

```bash
# Build for current platform
npm run build

# Build for all platforms
npm run build:all
```

## Features

- Visual configuration of all obfuscation parameters
- Drag-and-drop file input
- Real-time obfuscation progress
- Interactive report viewer
- Built-in diff viewer for before/after comparison
- Plugin management UI
- AI profiler visualization

## Screenshots

(Add screenshots here)

## Structure

```
gui/
├── src/
│   ├── main.ts          # Electron main process
│   ├── renderer.ts      # Renderer process
│   └── components/      # UI components
├── public/              # Static assets
└── dist/                # Build output
```
