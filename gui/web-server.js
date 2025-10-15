const http = require('http');
const fs = require('fs');
const path = require('path');
const { exec } = require('child_process');

const PORT = 3000;
const OBFUSCATOR_PATH = path.join(__dirname, '../build/obfuscator-cli');

const server = http.createServer((req, res) => {
  // Serve the main HTML page
  if (req.url === '/' || req.url === '/index.html') {
    fs.readFile(path.join(__dirname, 'public/index.html'), (err, data) => {
      if (err) {
        res.writeHead(500);
        res.end('Error loading index.html');
        return;
      }
      res.writeHead(200, { 'Content-Type': 'text/html' });
      res.end(data);
    });
  }
  // API endpoint to run obfuscator
  else if (req.url === '/api/obfuscate' && req.method === 'POST') {
    let body = '';
    req.on('data', chunk => {
      body += chunk.toString();
    });
    req.on('end', () => {
      try {
        const data = JSON.parse(body);
        const inputFile = data.inputFile || '../examples/simple_program.ll';
        const outputFile = data.outputFile || '/tmp/obfuscated.ll';
        const cycles = data.cycles || 1;
        
        const cmd = `${OBFUSCATOR_PATH} ${inputFile} -o ${outputFile} -cycles ${cycles} -v`;
        
        exec(cmd, { cwd: path.join(__dirname, '../examples') }, (error, stdout, stderr) => {
          const response = {
            success: !error,
            stdout: stdout,
            stderr: stderr,
            error: error ? error.message : null
          };
          
          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end(JSON.stringify(response));
        });
      } catch (err) {
        res.writeHead(400, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ success: false, error: err.message }));
      }
    });
  }
  // Get list of example files
  else if (req.url === '/api/examples') {
    const examplesDir = path.join(__dirname, '../examples');
    fs.readdir(examplesDir, (err, files) => {
      if (err) {
        res.writeHead(500, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ error: 'Failed to read examples' }));
        return;
      }
      const llFiles = files.filter(f => f.endsWith('.ll') || f.endsWith('.c'));
      res.writeHead(200, { 'Content-Type': 'application/json' });
      res.end(JSON.stringify({ files: llFiles }));
    });
  }
  else {
    res.writeHead(404);
    res.end('Not found');
  }
});

server.listen(PORT, () => {
  console.log(`\n🚀 Adaptive LLVM Obfuscator Web GUI`);
  console.log(`━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━`);
  console.log(`Server running at http://localhost:${PORT}`);
  console.log(`\nPress Ctrl+C to stop the server\n`);
});
