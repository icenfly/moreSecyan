document.addEventListener('DOMContentLoaded', () => {
    const connectBtn = document.getElementById('connectBtn');
    const executeBtn = document.getElementById('executeBtn');
    const connectionStatus = document.getElementById('connectionStatus');
    const serverLog = document.getElementById('serverLog');
    const clientLog = document.getElementById('clientLog');
    
    let serverRunning = false;
    let clientRunning = false;
    
    // Function to update the status with appropriate styling
    function updateStatus(message, type = '') {
        connectionStatus.textContent = message;
        connectionStatus.className = 'status ' + type;
    }
    
    // Function to append log messages
    function appendServerLog(message) {
        serverLog.textContent += message + '\n';
        serverLog.scrollTop = serverLog.scrollHeight;
    }
    
    function appendClientLog(message) {
        clientLog.textContent += message + '\n';
        clientLog.scrollTop = clientLog.scrollHeight;
    }
    
    // Connect button click handler
    connectBtn.addEventListener('click', async () => {
        updateStatus('Connecting...', 'info');
        connectBtn.disabled = true;
        
        // Clear previous logs
        serverLog.textContent = '';
        clientLog.textContent = '';
        
        try {
            // Start server process
            appendServerLog('Starting server process...');
            const serverResponse = await fetch('/api/start-server', {
                method: 'POST'
            });
            
            if (!serverResponse.ok) {
                throw new Error('Failed to start server process');
            }
            
            serverRunning = true;
            appendServerLog('Server process started successfully');
            appendServerLog('Waiting for connection...');
            
            // Wait a moment to ensure server is ready
            await new Promise(resolve => setTimeout(resolve, 1000));
            
            // Start client process
            appendClientLog('Starting client process...');
            const clientResponse = await fetch('/api/start-client', {
                method: 'POST'
            });
            
            if (!clientResponse.ok) {
                throw new Error('Failed to start client process');
            }
            
            clientRunning = true;
            appendClientLog('Client process started successfully');
            appendClientLog('Establishing connection...');
            
            // Wait for connection to be established
            const connectionCheckInterval = setInterval(async () => {
                const statusResponse = await fetch('/api/connection-status');
                const status = await statusResponse.json();
                
                if (status.connected) {
                    clearInterval(connectionCheckInterval);
                    updateStatus('Server and Client connected successfully!', 'success');
                    executeBtn.disabled = false;
                    appendServerLog('Connection established!');
                    appendClientLog('Connection established!');
                }
            }, 1000);
            
            // Set a timeout for connection
            setTimeout(() => {
                clearInterval(connectionCheckInterval);
                if (!executeBtn.disabled) return; // Already connected
                
                updateStatus('Connection timeout. Please try again.', 'error');
                connectBtn.disabled = false;
            }, 10000);
            
        } catch (error) {
            console.error('Connection error:', error);
            updateStatus(`Connection failed: ${error.message}`, 'error');
            connectBtn.disabled = false;
        }
    });
    
    // Execute button click handler
    executeBtn.addEventListener('click', async () => {
        if (!serverRunning || !clientRunning) {
            updateStatus('Server and Client must be connected first', 'error');
            return;
        }
        
        executeBtn.disabled = true;
        updateStatus('Executing query...', 'info');
        
        // Get user selected options
        const query = document.getElementById('query').value;
        const dataSize = document.getElementById('dataSize').value;
        const resultProtection = document.getElementById('resultProtection').value;
        const dualExecution = document.getElementById('dualExecution').value;
        
        try {
            // Send query parameters to backend
            const executeResponse = await fetch('/api/execute-query', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    query,
                    dataSize,
                    resultProtection,
                    dualExecution
                })
            });
            
            if (!executeResponse.ok) {
                throw new Error('Failed to execute query');
            }
            
            appendServerLog('\nExecuting query...');
            appendClientLog('\nExecuting query...');
            
            // Poll for results
            const resultCheckInterval = setInterval(async () => {
                const resultsResponse = await fetch('/api/query-results');
                const results = await resultsResponse.json();
                
                if (results.completed) {
                    clearInterval(resultCheckInterval);
                    
                    // Update server log
                    if (results.server) {
                        appendServerLog('\n--- Query Results ---');
                        appendServerLog(results.server);
                    }
                    
                    // Update client log
                    if (results.client) {
                        appendClientLog('\n--- Query Results ---');
                        appendClientLog(results.client);
                    }
                    
                    updateStatus('Query execution completed!', 'success');
                    executeBtn.disabled = false;
                }
            }, 1000);
            
            // Set a timeout for query execution
            setTimeout(() => {
                clearInterval(resultCheckInterval);
                if (executeBtn.disabled) {
                    updateStatus('Query execution taking longer than expected. Please check logs for status.', 'info');
                    executeBtn.disabled = false;
                }
            }, 60000); // 1 minute timeout
            
        } catch (error) {
            console.error('Execution error:', error);
            updateStatus(`Query execution failed: ${error.message}`, 'error');
            executeBtn.disabled = false;
        }
    });
}); 