/**
 * @file    app.js
 * @brief   Main Application Controller
 * @details Ties together parser and visualizer, handles user interactions
 */

// Global instances
const parser = new MapParser();
const visualizer = new MemoryVisualizer();

// DOM elements
let loadBtn, fileInput, searchBox;

/**
 * Initialize the application
 */
function init() {
    loadBtn = document.getElementById('loadBtn');
    fileInput = document.getElementById('fileInput');
    searchBox = document.getElementById('searchBox');
    
    // Event listeners
    loadBtn.addEventListener('click', () => fileInput.click());
    fileInput.addEventListener('change', handleFileSelect);
    searchBox.addEventListener('input', (e) => {
        visualizer.renderSymbols('symbolTable', e.target.value);
    });
    
    // Try to auto-load the map file
    autoLoadMapFile();
}

/**
 * Handle file selection
 */
function handleFileSelect(event) {
    const file = event.target.files[0];
    if (!file) return;
    
    const reader = new FileReader();
    reader.onload = (e) => {
        processMapFile(e.target.result);
    };
    reader.readAsText(file);
}

/**
 * Auto-load map file from default location
 */
async function autoLoadMapFile() {
    try {
        const response = await fetch('../../build/icarus_os.map');
        if (response.ok) {
            const content = await response.text();
            processMapFile(content);
            console.log('Auto-loaded map file from build/icarus_os.map');
        } else {
            showMessage('Map file not found. Click "Load Map File" to select manually.');
        }
    } catch (error) {
        console.log('Could not auto-load map file:', error.message);
        showMessage('Click "Load Map File" to get started.');
    }
}

/**
 * Process the map file content
 */
function processMapFile(content) {
    try {
        // Parse the map file
        const data = parser.parse(content);
        
        // Set data in visualizer
        visualizer.setData(data);
        
        // Get statistics
        const stats = parser.getStats();
        
        // Render everything
        visualizer.renderStats('stats', stats);
        visualizer.renderRegions('regionsList');
        visualizer.renderMemoryMap('memoryMap');
        visualizer.renderSymbols('symbolTable');
        
        showMessage('Map file loaded successfully!', 'success');
        
        // Log summary
        console.log('Parsed map file:');
        console.log('- Memory regions:', data.regions.length);
        console.log('- Sections:', data.sections.length);
        console.log('- Symbols:', data.symbols.length);
        console.log('- Flash usage:', stats.flashPercent + '%');
        console.log('- RAM usage:', stats.ramPercent + '%');
        
    } catch (error) {
        console.error('Error parsing map file:', error);
        showMessage('Error parsing map file: ' + error.message, 'error');
    }
}

/**
 * Show a message to the user
 */
function showMessage(message, type = 'info') {
    const statsDiv = document.getElementById('stats');
    if (!statsDiv) return;
    
    const colors = {
        info: '#667eea',
        success: '#28a745',
        error: '#dc3545'
    };
    
    const messageDiv = document.createElement('div');
    messageDiv.style.cssText = `
        padding: 10px 20px;
        background: white;
        border-radius: 8px;
        color: ${colors[type] || colors.info};
        font-weight: 500;
        box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
    `;
    messageDiv.textContent = message;
    
    // Replace stats temporarily
    const originalContent = statsDiv.innerHTML;
    statsDiv.innerHTML = '';
    statsDiv.appendChild(messageDiv);
    
    // Restore after 3 seconds if it's not an error
    if (type !== 'error' && originalContent) {
        setTimeout(() => {
            statsDiv.innerHTML = originalContent;
        }, 3000);
    }
}

/**
 * Export functionality for future use
 */
function exportData(format = 'json') {
    if (!parser.sections.length) {
        alert('No data to export. Load a map file first.');
        return;
    }
    
    const data = {
        regions: parser.memoryRegions,
        sections: parser.sections,
        symbols: parser.symbols,
        stats: parser.getStats()
    };
    
    const json = JSON.stringify(data, null, 2);
    const blob = new Blob([json], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    
    const a = document.createElement('a');
    a.href = url;
    a.download = 'memory_map_analysis.json';
    a.click();
    
    URL.revokeObjectURL(url);
}

/**
 * Close modal
 */
function closeModal() {
    const modal = document.getElementById('detailModal');
    if (modal) {
        modal.style.display = 'none';
    }
}

/**
 * Clear region filter
 */
function clearFilter() {
    visualizer.clearFilter();
}

/**
 * Clear symbol filter
 */
function clearSymbolFilter() {
    visualizer.clearSymbolFilter();
}

// Close modal when clicking outside
window.onclick = function(event) {
    const modal = document.getElementById('detailModal');
    if (event.target === modal) {
        closeModal();
    }
}

// Close modal with Escape key
document.addEventListener('keydown', function(event) {
    if (event.key === 'Escape') {
        closeModal();
    }
});

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
} else {
    init();
}
