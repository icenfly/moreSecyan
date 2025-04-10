const express = require('express');
const { spawn } = require('child_process');
const path = require('path');
const bodyParser = require('body-parser');
const app = express();
const port = 3000;

// Middleware
app.use(bodyParser.json());
app.use(express.static(__dirname)); // Serve static files from the current directory

// Store process and connection state
let serverProcess = null;
let clientProcess = null;
let isConnected = false;
let serverOutput = '';
let clientOutput = '';
let queryResults = {
    completed: false,
    server: '',
    client: ''
};

// Function to reset all state
function resetState() {
    if (serverProcess) {
        serverProcess.kill();
        serverProcess = null;
    }
    
    if (clientProcess) {
        clientProcess.kill();
        clientProcess = null;
    }
    
    isConnected = false;
    serverOutput = '';
    clientOutput = '';
    queryResults = {
        completed: false,
        server: '',
        client: ''
    };
}

// Get path to mpsi executable
const mpsiPath = path.resolve(__dirname, '../mpsi');

// API endpoint to start server process
app.post('/api/start-server', (req, res) => {
    try {
        // Reset state if any processes are running
        resetState();
        
        // Start server process
        serverProcess = spawn(mpsiPath);
        
        // Set up output handling
        serverProcess.stdout.on('data', (data) => {
            const output = data.toString();
            serverOutput += output;
            
            // Check for connection status
            if (output.includes('Establishing connection... Finished!')) {
                isConnected = true;
            }
            
            // Check for query results
            if (output.includes('Finished!') && output.includes('Communication cost:')) {
                queryResults.server = serverOutput;
                checkQueryCompletion();
            }
        });
        
        serverProcess.stderr.on('data', (data) => {
            serverOutput += `ERROR: ${data.toString()}`;
        });
        
        // Write "0" to the process to select server role
        serverProcess.stdin.write("0\n");
        
        // Send success response
        res.status(200).json({ success: true });
    } catch (error) {
        console.error('Failed to start server process:', error);
        res.status(500).json({ error: 'Failed to start server process' });
    }
});

// API endpoint to start client process
app.post('/api/start-client', (req, res) => {
    try {
        // Start client process
        clientProcess = spawn(mpsiPath);
        
        // Set up output handling
        clientProcess.stdout.on('data', (data) => {
            const output = data.toString();
            clientOutput += output;
            
            // Check for query results
            if (output.includes('Finished!') && output.includes('Communication cost:')) {
                queryResults.client = clientOutput;
                checkQueryCompletion();
            }
        });
        
        clientProcess.stderr.on('data', (data) => {
            clientOutput += `ERROR: ${data.toString()}`;
        });
        
        // Write "1" to the process to select client role
        clientProcess.stdin.write("1\n");
        
        // Send success response
        res.status(200).json({ success: true });
    } catch (error) {
        console.error('Failed to start client process:', error);
        res.status(500).json({ error: 'Failed to start client process' });
    }
});

// Function to check if both processes have completed
function checkQueryCompletion() {
    if (queryResults.server && queryResults.client) {
        queryResults.completed = true;
    }
}

// API endpoint to check connection status
app.get('/api/connection-status', (req, res) => {
    res.json({ connected: isConnected });
});

// API endpoint to execute query
app.post('/api/execute-query', (req, res) => {
    try {
        // Reset query results
        queryResults = {
            completed: false,
            server: '',
            client: ''
        };
        
        const { query, dataSize, resultProtection, dualExecution } = req.body;
        
        // Send query selection to server and client
        serverProcess.stdin.write(`${query}\n`);
        // Send data size selection
        serverProcess.stdin.write(`${dataSize}\n`);
        // Send result protection selection
        serverProcess.stdin.write(`${resultProtection}\n`);
        // Send dual execution selection
        serverProcess.stdin.write(`${dualExecution}\n`);

        clientProcess.stdin.write(`${query}\n`);
        clientProcess.stdin.write(`${dataSize}\n`);
        clientProcess.stdin.write(`${resultProtection}\n`);
        clientProcess.stdin.write(`${dualExecution}\n`);

        res.status(200).json({ success: true });
    } catch (error) {
        console.error('Failed to execute query:', error);
        res.status(500).json({ error: 'Failed to execute query' });
    }
});

// API endpoint to get query results
app.get('/api/query-results', (req, res) => {
    res.json(queryResults);
});

// Route for the main page
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

// Start the server
app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
}); 