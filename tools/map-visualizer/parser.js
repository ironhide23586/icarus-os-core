/**
 * @file    parser.js
 * @brief   Linker Map File Parser for STM32H750
 * @details Parses GNU LD map files and extracts memory regions, sections, and symbols
 */

class MapParser {
    constructor() {
        this.memoryRegions = [];
        this.sections = [];
        this.symbols = [];
    }

    /**
     * Parse the entire map file
     */
    parse(mapContent) {
        this.parseMemoryConfiguration(mapContent);
        this.parseLinkerScript(mapContent);
        this.parseSections(mapContent);
        this.parseSymbols(mapContent);
        
        return {
            regions: this.memoryRegions,
            sections: this.sections,
            symbols: this.symbols
        };
    }

    /**
     * Parse Memory Configuration section
     * Example:
     * Memory Configuration
     * Name             Origin             Length             Attributes
     * RAM_D1           0x24000000         0x00080000         xrw
     */
    parseMemoryConfiguration(content) {
        const memConfigRegex = /Memory Configuration\s+Name\s+Origin\s+Length\s+Attributes\s+([\s\S]*?)(?=\n\n|\nLinker script)/;
        const match = content.match(memConfigRegex);
        
        if (!match) return;
        
        const lines = match[1].split('\n');
        for (const line of lines) {
            const parts = line.trim().split(/\s+/);
            if (parts.length >= 4 && parts[0] !== '*default*') {
                const name = parts[0];
                const origin = parseInt(parts[1], 16);
                const length = parseInt(parts[2], 16);
                const attributes = parts[3];
                
                this.memoryRegions.push({
                    name,
                    origin,
                    length,
                    attributes,
                    used: 0,
                    sections: []
                });
            }
        }
    }

    /**
     * Parse Linker script and memory map section
     */
    parseLinkerScript(content) {
        const scriptRegex = /Linker script and memory map([\s\S]*?)(?=\nLOAD|$)/;
        const match = content.match(scriptRegex);
        
        if (!match) return;
        
        // This section contains detailed memory layout
        // We'll parse it for section information
    }

    /**
     * Parse section allocations
     * Example:
     * .text           0x08000000    0x12345
     *  *(.text)
     *  .text.main     0x08000100    0x234  obj/main.o
     */
    parseSections(content) {
        // Match section headers like ".text", ".data", ".bss", etc.
        const sectionRegex = /^(\.\w+(?:\.\w+)*)\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)/gm;
        
        let match;
        while ((match = sectionRegex.exec(content)) !== null) {
            const name = match[1];
            const address = parseInt(match[2], 16);
            const size = parseInt(match[3], 16);
            
            if (size > 0) {
                this.sections.push({
                    name,
                    address,
                    size,
                    end: address + size
                });
            }
        }
        
        // Sort by address
        this.sections.sort((a, b) => a.address - b.address);
        
        // Assign sections to memory regions
        for (const section of this.sections) {
            for (const region of this.memoryRegions) {
                if (section.address >= region.origin && 
                    section.address < region.origin + region.length) {
                    region.sections.push(section);
                    region.used += section.size;
                    section.region = region.name;
                    break;
                }
            }
        }
    }

    /**
     * Parse symbol table
     * Example formats:
     * 1. Section with symbol:
     *    .bss.g_game_state
     *                    0x240404cc       0x54 ./obj/Core/Src/game/game.o
     * 2. Direct symbol:
     *                    0x20004cd4                task_list
     * 3. Symbol with size:
     *    .text.main      0x08000100       0x234
     *                    0x08000100                main
     */
    parseSymbols(content) {
        const symbols = new Map(); // Use Map to deduplicate by address
        
        // Pattern 1: Section names that contain variable names
        // .bss.variable_name or .data.variable_name or .rodata.variable_name
        const sectionSymbolRegex = /^\s*\.(bss|data|rodata)\.(\w+)\s*$/gm;
        let match;
        
        while ((match = sectionSymbolRegex.exec(content)) !== null) {
            const symbolName = match[2];
            const startPos = match.index + match[0].length;
            
            // Look ahead for the address and size on the next line
            const nextLineMatch = content.substring(startPos, startPos + 200).match(/^\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)/m);
            if (nextLineMatch) {
                const address = parseInt(nextLineMatch[1], 16);
                const size = parseInt(nextLineMatch[2], 16);
                
                if (address > 0 && !symbols.has(address)) {
                    symbols.set(address, {
                        name: symbolName,
                        address,
                        size
                    });
                }
            }
        }
        
        // Pattern 2: Direct symbol definitions
        // Format: whitespace + address + whitespace + symbol_name
        const directSymbolRegex = /^\s+(0x[0-9a-f]+)\s+([a-zA-Z_][\w.]*)\s*$/gm;
        
        while ((match = directSymbolRegex.exec(content)) !== null) {
            const address = parseInt(match[1], 16);
            const name = match[2].trim();
            
            // Skip some common non-symbol entries
            if (name.startsWith('0x') || 
                name.includes('PROVIDE') || 
                name.length === 0 ||
                name === '.' ||
                name.startsWith('__')) {
                continue;
            }
            
            // Only add if we don't already have a symbol at this address
            if (address > 0 && !symbols.has(address)) {
                symbols.set(address, {
                    name,
                    address,
                    size: 0
                });
            }
        }
        
        // Pattern 3: Function/section symbols with explicit size
        // .text.function_name  0x08000100  0x234
        //                      0x08000100  function_name
        const sectionWithSizeRegex = /^\s*(\.\w+(?:\.\w+)*)\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)/gm;
        
        while ((match = sectionWithSizeRegex.exec(content)) !== null) {
            const sectionName = match[1];
            const address = parseInt(match[2], 16);
            const size = parseInt(match[3], 16);
            
            // Extract symbol name from section name (e.g., .text.main -> main)
            const parts = sectionName.split('.');
            if (parts.length >= 3) {
                const symbolName = parts.slice(2).join('.');
                
                if (address > 0 && size > 0 && !symbols.has(address)) {
                    symbols.set(address, {
                        name: symbolName,
                        address,
                        size
                    });
                }
            }
        }
        
        // Convert Map to array and sort by address
        this.symbols = Array.from(symbols.values()).sort((a, b) => a.address - b.address);
        
        // Calculate sizes for symbols that don't have them
        for (let i = 0; i < this.symbols.length - 1; i++) {
            const current = this.symbols[i];
            const next = this.symbols[i + 1];
            
            // Only calculate size if not already set and symbols are close (within 64KB)
            if (current.size === 0 && next.address - current.address < 0x10000) {
                current.size = next.address - current.address;
            }
        }
    }

    /**
     * Get statistics
     */
    getStats() {
        const totalFlash = this.memoryRegions.find(r => r.name.includes('FLASH'))?.length || 0;
        const usedFlash = this.memoryRegions.find(r => r.name.includes('FLASH'))?.used || 0;
        
        const totalRam = this.memoryRegions
            .filter(r => r.name.includes('RAM') || r.name.includes('DTCM'))
            .reduce((sum, r) => sum + r.length, 0);
        
        const usedRam = this.memoryRegions
            .filter(r => r.name.includes('RAM') || r.name.includes('DTCM'))
            .reduce((sum, r) => sum + r.used, 0);
        
        return {
            totalFlash,
            usedFlash,
            flashPercent: totalFlash > 0 ? (usedFlash / totalFlash * 100).toFixed(1) : 0,
            totalRam,
            usedRam,
            ramPercent: totalRam > 0 ? (usedRam / totalRam * 100).toFixed(1) : 0,
            numSections: this.sections.length,
            numSymbols: this.symbols.length
        };
    }
}
