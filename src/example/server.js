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
        try {
            // Try to send SIGTERM first for graceful shutdown
            serverProcess.kill('SIGTERM');
            // Force kill after a short delay if still running
            setTimeout(() => {
                if (serverProcess && !serverProcess.killed) {
                    serverProcess.kill('SIGKILL');
                }
            }, 500);
        } catch (err) {
            console.error('Error killing server process:', err);
        }
        serverProcess = null;
    }
    
    if (clientProcess) {
        try {
            // Try to send SIGTERM first for graceful shutdown
            clientProcess.kill('SIGTERM');
            // Force kill after a short delay if still running
            setTimeout(() => {
                if (clientProcess && !clientProcess.killed) {
                    clientProcess.kill('SIGKILL');
                }
            }, 500);
        } catch (err) {
            console.error('Error killing client process:', err);
        }
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
const mpsiPath = path.resolve(__dirname, 'mpsi');

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
            console.log('Server output:', output);
            
            // Check for connection status
            if (output.includes('Establishing connection... Finished!')) {
                isConnected = true;
                console.log('Server connection established');
            }
            
            // Check for query results
            if (output.includes('Finished!') && output.includes('Communication cost:')) {
                queryResults.server = serverOutput;
                checkQueryCompletion();
            }
        });
        
        serverProcess.stderr.on('data', (data) => {
            const errorOutput = `ERROR: ${data.toString()}`;
            serverOutput += errorOutput;
            console.error('Server error:', errorOutput);
        });
        
        serverProcess.on('error', (error) => {
            console.error('Server process error:', error);
            serverOutput += `Process error: ${error.message}\n`;
        });
        
        serverProcess.on('close', (code) => {
            console.log(`Server process exited with code ${code}`);
            serverOutput += `Process exited with code ${code}\n`;
            serverProcess = null;
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
            console.log('Client output:', output);
            
            // Check for query results
            if (output.includes('Finished!') && output.includes('Communication cost:')) {
                queryResults.client = clientOutput;
                checkQueryCompletion();
            }
        });
        
        clientProcess.stderr.on('data', (data) => {
            const errorOutput = `ERROR: ${data.toString()}`;
            clientOutput += errorOutput;
            console.error('Client error:', errorOutput);
        });
        
        clientProcess.on('error', (error) => {
            console.error('Client process error:', error);
            clientOutput += `Process error: ${error.message}\n`;
        });
        
        clientProcess.on('close', (code) => {
            console.log(`Client process exited with code ${code}`);
            clientOutput += `Process exited with code ${code}\n`;
            clientProcess = null;
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
    // Using regex to better detect completion patterns in the output
    const completionPattern = /Finished!.*Communication cost:/s;
    
    const serverCompleted = completionPattern.test(serverOutput);
    const clientCompleted = completionPattern.test(clientOutput);
    
    // Check if both server and client have received results
    if (serverCompleted && clientCompleted) {
        queryResults = {
            completed: true,
            server: serverOutput,
            client: clientOutput
        };
        
        console.log('Query completed successfully');
    }
}

// API endpoint to check connection status
app.get('/api/connection-status', (req, res) => {
    res.json({ connected: isConnected });
});

// API endpoint to execute query
app.post('/api/execute-query', (req, res) => {
    try {
        if (!serverProcess || !clientProcess || !isConnected) {
            return res.status(400).json({ 
                error: 'Server and client must be connected before executing a query',
                serverActive: !!serverProcess,
                clientActive: !!clientProcess,
                connected: isConnected
            });
        }
        
        // Reset query results and output logs for the new query
        queryResults = {
            completed: false,
            server: '',
            client: ''
        };
        
        // Keep the historical logs but mark where the new query begins
        serverOutput += '\n--- NEW QUERY ---\n';
        clientOutput += '\n--- NEW QUERY ---\n';
        
        const { query, dataSize, resultProtection, dualExecution } = req.body;
        
        // Validate input parameters
        if (query === undefined || dataSize === undefined || 
            resultProtection === undefined || dualExecution === undefined) {
            return res.status(400).json({ error: 'Missing required query parameters' });
        }
        
        console.log(`Executing query with parameters: query=${query}, dataSize=${dataSize}, resultProtection=${resultProtection}, dualExecution=${dualExecution}`);
        
        // Use promises to handle the write operations
        const writeToServer = new Promise((resolve, reject) => {
            serverProcess.stdin.write(`${query}\n`, (err) => {
                if (err) reject(err);
                serverProcess.stdin.write(`${dataSize}\n`, (err) => {
                    if (err) reject(err);
                    serverProcess.stdin.write(`${resultProtection}\n`, (err) => {
                        if (err) reject(err);
                        serverProcess.stdin.write(`${dualExecution}\n`, (err) => {
                            if (err) reject(err);
                            resolve();
                        });
                    });
                });
            });
        });
        
        const writeToClient = new Promise((resolve, reject) => {
            clientProcess.stdin.write(`${query}\n`, (err) => {
                if (err) reject(err);
                clientProcess.stdin.write(`${dataSize}\n`, (err) => {
                    if (err) reject(err);
                    clientProcess.stdin.write(`${resultProtection}\n`, (err) => {
                        if (err) reject(err);
                        clientProcess.stdin.write(`${dualExecution}\n`, (err) => {
                            if (err) reject(err);
                            resolve();
                        });
                    });
                });
            });
        });
        
        // Execute both write operations in parallel
        Promise.all([writeToServer, writeToClient])
            .then(() => {
                res.status(200).json({ success: true });
            })
            .catch((error) => {
                console.error('Error writing to processes:', error);
                res.status(500).json({ error: 'Failed to send query parameters to processes' });
            });
    } catch (error) {
        console.error('Failed to execute query:', error);
        res.status(500).json({ error: 'Failed to execute query' });
    }
});

// API endpoint to get query results
app.get('/api/query-results', (req, res) => {
    res.json(queryResults);
});

// API endpoint to reset state
app.post('/api/reset-state', (req, res) => {
    try {
        resetState();
        res.status(200).json({ success: true });
    } catch (error) {
        console.error('Failed to reset state:', error);
        res.status(500).json({ error: 'Failed to reset state' });
    }
});

// Route for the main page
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

// Start the server
app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
}); 