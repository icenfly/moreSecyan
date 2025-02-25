document.addEventListener('DOMContentLoaded', function() {
    const form = document.getElementById('psi-form');
    const resultsElement = document.getElementById('results');
    const statusLight = document.getElementById('status-light');
    const statusText = document.getElementById('status-text');
    const runButton = document.getElementById('run-button');

    // 设置状态函数
    function setStatus(status, message) {
        statusLight.className = 'status-light ' + status;
        statusText.textContent = message;
        
        if (status === 'running') {
            runButton.disabled = true;
        } else {
            runButton.disabled = false;
        }
    }

    // 处理表单提交
    form.addEventListener('submit', function(event) {
        event.preventDefault();
        
        // 获取表单数据
        const formData = new FormData(form);
        const params = new URLSearchParams();
        
        for (const [key, value] of formData.entries()) {
            params.append(key, value);
        }
        
        // 更新状态
        setStatus('running', '正在运行...');
        resultsElement.textContent = '正在处理请求，请稍候...';
        
        // 发送API请求
        fetch('/api/run_psi', {
            method: 'POST',
            body: params,
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            }
        })
        .then(response => {
            if (!response.ok) {
                throw new Error('网络响应错误');
            }
            return response.text();
        })
        .then(data => {
            // 显示结果
            resultsElement.textContent = data;
            setStatus('success', '完成');
        })
        .catch(error => {
            // 显示错误
            resultsElement.textContent = '错误: ' + error.message;
            setStatus('error', '错误');
            console.error('Error:', error);
        });
    });

    // 添加角色选择说明
    const roleSelect = document.getElementById('role');
    roleSelect.addEventListener('change', function() {
        const roleValue = roleSelect.value;
        let warningMessage = '';
        
        if (roleValue === '0') {
            warningMessage = '注意: 选择服务器角色时，请确保客户端已经准备好连接。';
        } else {
            warningMessage = '注意: 选择客户端角色时，请确保服务器已经启动并监听连接。';
        }
        
        // 显示警告消息
        if (!document.getElementById('role-warning')) {
            const warningElement = document.createElement('div');
            warningElement.id = 'role-warning';
            warningElement.className = 'warning';
            warningElement.textContent = warningMessage;
            roleSelect.parentNode.appendChild(warningElement);
        } else {
            document.getElementById('role-warning').textContent = warningMessage;
        }
    });

    // 初始状态
    setStatus('idle', '空闲');
}); 