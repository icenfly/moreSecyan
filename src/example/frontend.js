document.addEventListener('DOMContentLoaded', () => {
    const connectBtn = document.getElementById('connectBtn');
    const executeBtn = document.getElementById('executeBtn');
    const connectionStatus = document.getElementById('connectionStatus');
    const serverLog = document.getElementById('serverLog');
    const clientLog = document.getElementById('clientLog');
    const resultsContainer = document.getElementById('resultsContainer');
    const resultTable = document.getElementById('resultTable');
    const runningTime = document.getElementById('runningTime');
    const commCost = document.getElementById('commCost');
    const emptyResultsMessage = document.getElementById('emptyResultsMessage');
    
    let serverRunning = false;
    let clientRunning = false;
    let connectionTimeoutId = null;
    let queryTimeoutId = null;
    
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

    // Function to parse and display query results
    function displayQueryResults(resultText) {
        if (!resultText) {
            // Reset display state
            emptyResultsMessage.style.display = 'block';
            resultTable.style.display = 'none';
            runningTime.textContent = '-';
            commCost.textContent = '-';
            return;
        }
        
        try {
            // Reset the table and metrics
            resultTable.innerHTML = '';
            runningTime.textContent = '-';
            commCost.textContent = '-';
            
            // Regular expressions to extract data
            const tableRegex = /^(row_num\t.*(?:\n\d+\t.*)*)$/m;
            const runningTimeRegex = /Running time: (\d+)ms/;
            const commCostRegex = /Communication cost: ([\d\.]+) MB/;
            
            // Extract table data
            const tableMatch = resultText.match(tableRegex);
            if (tableMatch && tableMatch[1]) {
                const tableLines = tableMatch[1].split('\n');
                
                // Create table header
                const headerRow = document.createElement('tr');
                const headerCells = tableLines[0].split('\t');
                headerCells.forEach(header => {
                    const th = document.createElement('th');
                    th.textContent = header;
                    headerRow.appendChild(th);
                });
                resultTable.appendChild(headerRow);
                
                // Create table rows
                for (let i = 1; i < tableLines.length; i++) {
                    const dataRow = document.createElement('tr');
                    const dataCells = tableLines[i].split('\t');
                    dataCells.forEach(cell => {
                        const td = document.createElement('td');
                        td.textContent = cell;
                        dataRow.appendChild(td);
                    });
                    resultTable.appendChild(dataRow);
                }
                
                // Show the table and hide the empty message
                emptyResultsMessage.style.display = 'none';
                resultTable.style.display = 'table';
            } else {
                // No table data found
                emptyResultsMessage.style.display = 'block';
                resultTable.style.display = 'none';
            }
            
            // Extract metrics
            const timeMatch = resultText.match(runningTimeRegex);
            if (timeMatch && timeMatch[1]) {
                runningTime.textContent = timeMatch[1] + ' 毫秒';
            }
            
            const costMatch = resultText.match(commCostRegex);
            if (costMatch && costMatch[1]) {
                commCost.textContent = costMatch[1] + ' MB';
            }
            
        } catch (error) {
            console.error('解析查询结果时出错:', error);
            emptyResultsMessage.style.display = 'block';
            resultTable.style.display = 'none';
        }
    }
    
    // Connect button click handler
    connectBtn.addEventListener('click', async () => {
        updateStatus('正在连接...', 'info');
        connectBtn.disabled = true;
        
        // Clear any existing timeouts
        if (connectionTimeoutId) {
            clearTimeout(connectionTimeoutId);
            connectionTimeoutId = null;
        }
        
        // Clear previous logs
        serverLog.textContent = '';
        clientLog.textContent = '';
        
        // Reset results display
        displayQueryResults(null);
        
        try {
            // Start server process
            appendServerLog('正在启动服务器进程...');
            const serverResponse = await fetch('/api/start-server', {
                method: 'POST'
            });
            
            if (!serverResponse.ok) {
                throw new Error('启动服务器进程失败');
            }
            
            serverRunning = true;
            appendServerLog('服务器进程启动成功');
            appendServerLog('等待连接...');
            
            // Wait a moment to ensure server is ready
            await new Promise(resolve => setTimeout(resolve, 1000));
            
            // Start client process
            appendClientLog('正在启动客户端进程...');
            const clientResponse = await fetch('/api/start-client', {
                method: 'POST'
            });
            
            if (!clientResponse.ok) {
                throw new Error('启动客户端进程失败');
            }
            
            clientRunning = true;
            appendClientLog('客户端进程启动成功');
            appendClientLog('正在建立连接...');
            
            // Wait for connection to be established
            const connectionCheckInterval = setInterval(async () => {
                const statusResponse = await fetch('/api/connection-status');
                const status = await statusResponse.json();
                
                if (status.connected) {
                    clearInterval(connectionCheckInterval);
                    
                    // Clear the timeout once connected
                    if (connectionTimeoutId) {
                        clearTimeout(connectionTimeoutId);
                        connectionTimeoutId = null;
                    }
                    
                    updateStatus('服务器和客户端连接成功！', 'success');
                    executeBtn.disabled = false;
                    appendServerLog('连接已建立！');
                    appendClientLog('连接已建立！');
                }
            }, 1000);
            
            // Set a timeout for connection
            connectionTimeoutId = setTimeout(() => {
                clearInterval(connectionCheckInterval);
                if (!executeBtn.disabled) return; // Already connected
                
                updateStatus('连接超时。请重试。', 'error');
                connectBtn.disabled = false;
            }, 10000);
            
        } catch (error) {
            console.error('连接错误:', error);
            updateStatus(`连接失败: ${error.message}`, 'error');
            connectBtn.disabled = false;
        }
    });
    
    // Execute button click handler
    executeBtn.addEventListener('click', async () => {
        if (!serverRunning || !clientRunning) {
            updateStatus('必须先连接服务器和客户端', 'error');
            return;
        }
        
        // Clear any existing timeouts
        if (queryTimeoutId) {
            clearTimeout(queryTimeoutId);
            queryTimeoutId = null;
        }
        
        executeBtn.disabled = true;
        updateStatus('正在执行查询...', 'info');
        
        // Reset results display
        displayQueryResults(null);
        
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
                throw new Error('执行查询失败');
            }
            
            appendServerLog('\n正在执行查询...');
            appendClientLog('\n正在执行查询...');
            
            let queryCompleted = false;
            
            // Poll for results
            const resultCheckInterval = setInterval(async () => {
                try {
                    const resultsResponse = await fetch('/api/query-results');
                    const results = await resultsResponse.json();
                    
                    if (results.completed) {
                        clearInterval(resultCheckInterval);
                        queryCompleted = true;
                        
                        // Clear the timeout once completed
                        if (queryTimeoutId) {
                            clearTimeout(queryTimeoutId);
                            queryTimeoutId = null;
                        }
                        
                        // Update server log
                        if (results.server) {
                            appendServerLog('\n--- 查询结果 ---');
                            appendServerLog(results.server);
                        }
                        
                        // Update client log
                        if (results.client) {
                            appendClientLog('\n--- 查询结果 ---');
                            appendClientLog(results.client);
                            
                            // Parse and display the structured results
                            displayQueryResults(results.client);
                        }
                        
                        updateStatus('查询执行完成！请重新连接以运行另一个查询。', 'success');
                        
                        // UPDATED BEHAVIOR: Reset connection state after query execution
                        serverRunning = false;
                        clientRunning = false;
                        executeBtn.disabled = true;
                        connectBtn.disabled = false;  // Re-enable Connect button
                    }
                } catch (error) {
                    console.error('检查查询结果时出错:', error);
                    // Don't clear the interval, keep trying
                }
            }, 1000);
            
            // Set a timeout for query execution
            queryTimeoutId = setTimeout(() => {
                if (!queryCompleted) {
                    clearInterval(resultCheckInterval);
                    updateStatus('查询执行时间超过预期。请查看日志了解状态。', 'info');
                    executeBtn.disabled = false;
                }
            }, 60000); // 1 minute timeout
            
        } catch (error) {
            console.error('执行错误:', error);
            updateStatus(`查询执行失败: ${error.message}`, 'error');
            executeBtn.disabled = false;
        }
    });
}); 