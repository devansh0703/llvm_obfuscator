import { app, BrowserWindow, ipcMain, dialog, IpcMainInvokeEvent } from 'electron';
import { spawn } from 'node:child_process';
import * as path from 'node:path';
import * as fs from 'node:fs';

let mainWindow: BrowserWindow | null = null;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1200,
    height: 800,
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: false
    }
  });

  mainWindow.loadFile(path.join(__dirname, '../public/index.html'));

  // Open DevTools in development
  if (process.env.NODE_ENV === 'development') {
    mainWindow.webContents.openDevTools();
  }

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
}

app.on('ready', createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (mainWindow === null) {
    createWindow();
  }
});

// IPC handlers
ipcMain.handle('select-file', async () => {
  const result = await dialog.showOpenDialog({
    properties: ['openFile'],
    filters: [
      { name: 'LLVM IR', extensions: ['ll'] },
      { name: 'LLVM Bitcode', extensions: ['bc'] },
      { name: 'All Files', extensions: ['*'] }
    ]
  });
  
  return result.filePaths[0];
});

interface ObfuscateConfig {
  inputFile: string;
  outputFile: string;
  cycles: number;
  watermark?: string;
}

ipcMain.handle('obfuscate', async (event: IpcMainInvokeEvent, config: ObfuscateConfig) => {
  return new Promise((resolve, reject) => {
    const args = [
      config.inputFile,
      '-o', config.outputFile,
      '-cycles', config.cycles.toString()
    ];
    
    if (config.watermark) {
      args.push('--watermark', config.watermark);
    }
    
    const obfuscator = spawn('./obfuscator-cli', args);
    
    let output = '';
    let error = '';
    
    obfuscator.stdout?.on('data', (data: Buffer) => {
      output += data.toString();
      event.sender.send('obfuscation-progress', data.toString());
    });
    
    obfuscator.stderr?.on('data', (data: Buffer) => {
      error += data.toString();
    });
    
    obfuscator.on('close', (code: number | null) => {
      if (code === 0) {
        resolve({ success: true, output });
      } else {
        reject({ success: false, error });
      }
    });
  });
});

ipcMain.handle('load-report', async (_event: IpcMainInvokeEvent, reportPath: string) => {
  try {
    const data = fs.readFileSync(reportPath, 'utf-8');
    return JSON.parse(data);
  } catch (error) {
    console.error('Failed to load report:', error);
    return null;
  }
});
