document.addEventListener('DOMContentLoaded', function() {
    // 角色选择
    const serverBtn = document.getElementById('server-btn');
    const clientBtn = document.getElementById('client-btn');
    let selectedRole = null;

    serverBtn.addEventListener('click', function() {
        selectedRole = 0; // 服务器角色
        serverBtn.classList.add('active');
        clientBtn.classList.remove('active');
    });

    clientBtn.addEventListener('click', function() {
        selectedRole = 1; // 客户端角色
        clientBtn.classList.add('active');
        serverBtn.classList.remove('active');
    });

    // 表单提交
    const queryForm = document.getElementById('query-form');
    const statusMessage = document.getElementById('status-message');
    const resultsOutput = document.getElementById('results-output');
    const runningTimeDiv = document.getElementById('running-time');
    const commCostDiv = document.getElementById('comm-cost');

    queryForm.addEventListener('submit', function(e) {
        e.preventDefault();
        
        if (selectedRole === null) {
            showStatus('请先选择角色（服务器或客户端）', 'error');
            return;
        }
        
        const queryType = document.getElementById('query-type').value;
        const dataSize = document.getElementById('data-size').value;
        const resultProtection = document.getElementById('result-protection').checked ? 1 : 0;
        const serverAddress = document.getElementById('server-address').value;
        const serverPort = document.getElementById('server-port').value;
        
        // 清空之前的结果
        resultsOutput.textContent = '';
        runningTimeDiv.textContent = '';
        commCostDiv.textContent = '';
        
        // 显示正在处理的状态
        showStatus('正在连接并执行查询，请稍候...', 'info');
        
        // 发送请求到后端
        fetch('/run_query', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                role: selectedRole,
                query_type: parseInt(queryType),
                data_size: parseInt(dataSize),
                result_protection: resultProtection,
                server_address: serverAddress,
                server_port: parseInt(serverPort)
            })
        })
        .then(response => {
            if (!response.ok) {
                throw new Error('网络响应异常');
            }
            return response.json();
        })
        .then(data => {
            if (data.status === 'success') {
                showStatus('查询执行成功！', 'success');
                
                // 显示结果
                resultsOutput.textContent = data.results;
                
                // 显示性能指标
                if (data.running_time) {
                    runningTimeDiv.textContent = `运行时间: ${data.running_time}`;
                }
                
                if (data.comm_cost) {
                    commCostDiv.textContent = `通信成本: ${data.comm_cost} MB`;
                }
            } else {
                showStatus(`查询执行失败: ${data.error}`, 'error');
            }
        })
        .catch(error => {
            showStatus(`发生错误: ${error.message}`, 'error');
        });
    });
    
    // 辅助函数：显示状态消息
    function showStatus(message, type) {
        statusMessage.textContent = message;
        statusMessage.className = 'status';
        statusMessage.classList.add(type);
    }
}); 