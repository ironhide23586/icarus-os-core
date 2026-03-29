/**
 * @file    visualizer.js
 * @brief   Memory Map Visualization Engine
 * @details Renders memory regions, sections, and symbols visually
 */

class MemoryVisualizer {
    constructor() {
        this.data = null;
        this.selectedRegion = null;
        this.selectedSection = null;
        this.selectedSymbol = null;
    }

    /**
     * Set the parsed data
     */
    setData(data) {
        this.data = data;
    }

    /**
     * Render memory regions list
     */
    renderRegions(containerId) {
        const container = document.getElementById(containerId);
        if (!container || !this.data) return;
        
        container.innerHTML = '';
        
        for (const region of this.data.regions) {
            const card = document.createElement('div');
            card.className = 'region-card';
            if (this.selectedRegion === region) {
                card.classList.add('selected');
            }
            card.onclick = () => this.selectRegion(region);
            
            const usagePercent = region.length > 0 ? (region.used / region.length * 100).toFixed(1) : 0;
            
            card.innerHTML = `
                <div class="region-name">${region.name}</div>
                <div class="region-info">
                    Origin: 0x${region.origin.toString(16).toUpperCase().padStart(8, '0')}<br>
                    Size: ${this.formatSize(region.length)}<br>
                    Used: ${this.formatSize(region.used)} (${usagePercent}%)<br>
                    Attrs: ${region.attributes}
                </div>
            `;
            
            container.appendChild(card);
        }
    }

    /**
     * Select a region to highlight
     */
    selectRegion(region) {
        this.selectedRegion = region;
        this.selectedSection = null;
        this.renderRegions('regionsList');
        this.renderMemoryMap('memoryMap');
        this.updateFilterIndicator();
    }

    /**
     * Render memory map visualization
     */
    renderMemoryMap(containerId) {
        const container = document.getElementById(containerId);
        if (!container || !this.data) return;
        
        container.innerHTML = '';
        
        // Determine which sections to show
        let sectionsToShow = this.data.sections;
        if (this.selectedRegion) {
            sectionsToShow = this.selectedRegion.sections;
        }
        
        if (sectionsToShow.length === 0) {
            container.innerHTML = '<p style="color: #6c757d; text-align: center; padding: 40px;">No sections to display. Click a memory region to view its contents.</p>';
            return;
        }
        
        // Calculate total size for proportional rendering
        const totalSize = sectionsToShow.reduce((sum, s) => sum + s.size, 0);
        
        // Create blocks
        for (const section of sectionsToShow) {
            const block = document.createElement('div');
            block.className = 'memory-block ' + this.getColorClass(section.name);
            
            if (this.selectedSection === section) {
                block.classList.add('selected');
            }
            
            // Calculate height proportional to size (min 35px, max 250px)
            const heightPercent = (section.size / totalSize) * 100;
            const height = Math.max(35, Math.min(250, heightPercent * 5));
            block.style.height = height + 'px';
            
            block.innerHTML = `
                <span class="block-label">${section.name}</span>
                <span class="block-size">${this.formatSize(section.size)}</span>
            `;
            
            // Click handler to show section details and filter symbols
            block.onclick = () => this.selectSection(section);
            
            container.appendChild(block);
        }
    }

    /**
     * Select a section and show its symbols
     */
    selectSection(section) {
        this.selectedSection = section;
        this.renderMemoryMap('memoryMap');
        
        // Filter symbols in this section
        const sectionSymbols = this.data.symbols.filter(s => 
            s.address >= section.address && s.address < section.end
        );
        
        // Update symbol filter indicator
        const indicator = document.getElementById('symbolFilterIndicator');
        const text = document.getElementById('symbolFilterText');
        if (indicator && text) {
            indicator.classList.add('active');
            text.textContent = `Showing ${sectionSymbols.length} symbols in ${section.name}`;
        }
        
        // Render filtered symbols
        this.renderSymbols('symbolTable', '', sectionSymbols);
        
        // Show section details modal
        this.showSectionModal(section, sectionSymbols);
    }

    /**
     * Update filter indicator
     */
    updateFilterIndicator() {
        const indicator = document.getElementById('filterIndicator');
        const text = document.getElementById('filterText');
        
        if (indicator && text) {
            if (this.selectedRegion) {
                indicator.classList.add('active');
                text.textContent = `Filtering: ${this.selectedRegion.name} (${this.selectedRegion.sections.length} sections)`;
            } else {
                indicator.classList.remove('active');
            }
        }
    }

    /**
     * Clear region filter
     */
    clearFilter() {
        this.selectedRegion = null;
        this.selectedSection = null;
        this.renderRegions('regionsList');
        this.renderMemoryMap('memoryMap');
        this.updateFilterIndicator();
    }

    /**
     * Clear symbol filter
     */
    clearSymbolFilter() {
        this.selectedSection = null;
        this.renderMemoryMap('memoryMap');
        
        const indicator = document.getElementById('symbolFilterIndicator');
        if (indicator) {
            indicator.classList.remove('active');
        }
        
        this.renderSymbols('symbolTable', '');
    }

    /**
     * Render symbol table
     */
    renderSymbols(containerId, searchTerm = '', customSymbols = null) {
        const container = document.getElementById(containerId);
        if (!container || !this.data) return;
        
        container.innerHTML = '';
        
        // Use custom symbols if provided, otherwise use all symbols
        let symbols = customSymbols || this.data.symbols;
        
        // Filter by search term
        if (searchTerm) {
            const term = searchTerm.toLowerCase();
            symbols = symbols.filter(s => s.name.toLowerCase().includes(term));
        }
        
        // Limit to first 500 for performance
        const displaySymbols = symbols.slice(0, 500);
        
        if (displaySymbols.length === 0) {
            container.innerHTML = '<p style="color: #6c757d; padding: 20px; text-align: center;">No symbols found.</p>';
            return;
        }
        
        for (const symbol of displaySymbols) {
            const row = document.createElement('div');
            row.className = 'symbol-row';
            
            if (this.selectedSymbol === symbol) {
                row.classList.add('selected');
            }
            
            const sizeStr = symbol.size > 0 ? this.formatSize(symbol.size) : '';
            
            row.innerHTML = `
                <span class="symbol-name" title="${this.escapeHtml(symbol.name)}">${this.escapeHtml(symbol.name)}</span>
                <span class="symbol-addr">@ 0x${symbol.address.toString(16).toUpperCase().padStart(8, '0')}</span>
                ${sizeStr ? `<span class="symbol-size">${sizeStr}</span>` : ''}
            `;
            
            // Click handler to show symbol details
            row.onclick = () => this.selectSymbol(symbol);
            
            container.appendChild(row);
        }
        
        if (symbols.length > 500) {
            const note = document.createElement('p');
            note.style.cssText = 'color: #6c757d; padding: 15px; font-style: italic; text-align: center; background: #f8f9fa; border-radius: 8px; margin-top: 10px;';
            note.textContent = `Showing first 500 of ${symbols.length} symbols. Use search to narrow results.`;
            container.appendChild(note);
        }
    }

    /**
     * Select a symbol and show details
     */
    selectSymbol(symbol) {
        this.selectedSymbol = symbol;
        this.showSymbolModal(symbol);
    }

    /**
     * Show symbol details modal
     */
    showSymbolModal(symbol) {
        const modal = document.getElementById('detailModal');
        const title = document.getElementById('modalTitle');
        const body = document.getElementById('modalBody');
        
        if (!modal || !title || !body) return;
        
        title.textContent = 'Symbol Details';
        
        // Find which section this symbol belongs to
        let section = null;
        for (const sec of this.data.sections) {
            if (symbol.address >= sec.address && symbol.address < sec.end) {
                section = sec;
                break;
            }
        }
        
        const sectionBadge = section ? 
            `<span class="badge ${this.getBadgeClass(section.name)}">${section.name}</span>` : 
            '<span class="badge" style="background: #6c757d; color: white;">Unknown</span>';
        
        body.innerHTML = `
            <div class="info-grid">
                <div class="info-item">
                    <div class="info-label">Symbol Name</div>
                    <div class="info-value" style="font-size: 1em; word-break: break-all;">${this.escapeHtml(symbol.name)}</div>
                </div>
                <div class="info-item">
                    <div class="info-label">Address</div>
                    <div class="info-value">0x${symbol.address.toString(16).toUpperCase().padStart(8, '0')}</div>
                </div>
                <div class="info-item">
                    <div class="info-label">Size</div>
                    <div class="info-value">${symbol.size > 0 ? this.formatSize(symbol.size) : 'Unknown'}</div>
                </div>
                <div class="info-item">
                    <div class="info-label">Section</div>
                    <div class="info-value" style="font-size: 0.9em;">${sectionBadge}</div>
                </div>
            </div>
            
            ${section ? `
                <div class="section-header">Section Information</div>
                <div class="info-grid">
                    <div class="info-item">
                        <div class="info-label">Section Address</div>
                        <div class="info-value">0x${section.address.toString(16).toUpperCase().padStart(8, '0')}</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Section Size</div>
                        <div class="info-value">${this.formatSize(section.size)}</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Memory Region</div>
                        <div class="info-value" style="font-size: 0.9em;">${section.region || 'Unknown'}</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">Offset in Section</div>
                        <div class="info-value">+${(symbol.address - section.address).toString(16).toUpperCase()}</div>
                    </div>
                </div>
            ` : ''}
        `;
        
        modal.style.display = 'block';
    }

    /**
     * Show section details modal
     */
    showSectionModal(section, symbols) {
        const modal = document.getElementById('detailModal');
        const title = document.getElementById('modalTitle');
        const body = document.getElementById('modalBody');
        
        if (!modal || !title || !body) return;
        
        title.textContent = 'Section Details';
        
        const sectionBadge = `<span class="badge ${this.getBadgeClass(section.name)}">${section.name}</span>`;
        
        let symbolsHtml = '';
        if (symbols.length > 0) {
            symbolsHtml = `
                <div class="section-header">Symbols in this Section (${symbols.length})</div>
                <div class="symbol-list-modal">
                    ${symbols.slice(0, 50).map(s => `
                        <div class="symbol-item-modal">
                            <strong>${this.escapeHtml(s.name)}</strong><br>
                            <span style="color: #6c757d;">0x${s.address.toString(16).toUpperCase().padStart(8, '0')}</span>
                            ${s.size > 0 ? `<span style="color: #28a745; margin-left: 10px;">${this.formatSize(s.size)}</span>` : ''}
                        </div>
                    `).join('')}
                    ${symbols.length > 50 ? `<p style="text-align: center; color: #6c757d; padding: 10px;">Showing first 50 of ${symbols.length} symbols</p>` : ''}
                </div>
            `;
        }
        
        body.innerHTML = `
            <div class="info-grid">
                <div class="info-item">
                    <div class="info-label">Section Name</div>
                    <div class="info-value" style="font-size: 1em;">${sectionBadge}</div>
                </div>
                <div class="info-item">
                    <div class="info-label">Start Address</div>
                    <div class="info-value">0x${section.address.toString(16).toUpperCase().padStart(8, '0')}</div>
                </div>
                <div class="info-item">
                    <div class="info-label">End Address</div>
                    <div class="info-value">0x${section.end.toString(16).toUpperCase().padStart(8, '0')}</div>
                </div>
                <div class="info-item">
                    <div class="info-label">Size</div>
                    <div class="info-value">${this.formatSize(section.size)}</div>
                </div>
                <div class="info-item">
                    <div class="info-label">Memory Region</div>
                    <div class="info-value" style="font-size: 0.9em;">${section.region || 'Unknown'}</div>
                </div>
                <div class="info-item">
                    <div class="info-label">Symbol Count</div>
                    <div class="info-value">${symbols.length}</div>
                </div>
            </div>
            
            ${symbolsHtml}
        `;
        
        modal.style.display = 'block';
    }

    /**
     * Get badge class for section type
     */
    getBadgeClass(sectionName) {
        const name = sectionName.toLowerCase();
        
        if (name.includes('flash') || name.includes('.text')) return 'badge-flash';
        if (name.includes('dtcm')) return 'badge-dtcm';
        if (name.includes('itcm')) return 'badge-itcm';
        if (name.includes('stack')) return 'badge-stack';
        if (name.includes('.bss')) return 'badge-bss';
        if (name.includes('.data')) return 'badge-data';
        if (name.includes('ram')) return 'badge-ram';
        
        return 'badge-ram';
    }

    /**
     * Render statistics
     */
    renderStats(containerId, stats) {
        const container = document.getElementById(containerId);
        if (!container) return;
        
        container.innerHTML = `
            <div class="stat-item">
                <div class="stat-label">Flash</div>
                <div class="stat-value">${stats.flashPercent}%</div>
            </div>
            <div class="stat-item">
                <div class="stat-label">RAM</div>
                <div class="stat-value">${stats.ramPercent}%</div>
            </div>
            <div class="stat-item">
                <div class="stat-label">Sections</div>
                <div class="stat-value">${stats.numSections}</div>
            </div>
            <div class="stat-item">
                <div class="stat-label">Symbols</div>
                <div class="stat-value">${stats.numSymbols}</div>
            </div>
        `;
    }

    /**
     * Get color class based on section name
     */
    getColorClass(sectionName) {
        const name = sectionName.toLowerCase();
        
        if (name.includes('flash') || name.includes('.text')) return 'color-flash';
        if (name.includes('dtcm')) return 'color-dtcm';
        if (name.includes('itcm')) return 'color-itcm';
        if (name.includes('stack')) return 'color-stack';
        if (name.includes('.bss')) return 'color-bss';
        if (name.includes('.data')) return 'color-data';
        if (name.includes('ram')) return 'color-ram';
        
        return 'color-ram';  // Default
    }

    /**
     * Format byte size to human readable
     */
    formatSize(bytes) {
        if (bytes === 0) return '0 B';
        if (bytes < 1024) return bytes + ' B';
        if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
        return (bytes / (1024 * 1024)).toFixed(2) + ' MB';
    }

    /**
     * Escape HTML for safe rendering
     */
    escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }
}
