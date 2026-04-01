# ICARUS Memory Map Visualizer

Beautiful, interactive web-based tool for analyzing and visualizing STM32H750 linker map files.

## Features

- **Automatic Loading**: Automatically loads `build/icarus_os.map` when opened
- **Interactive Memory Regions**: Click regions to filter sections
- **Interactive Memory Blocks**: Click sections to view symbols and details
- **Interactive Symbols**: Click symbols to see detailed information
- **Visual Layout**: Proportional visualization with smooth animations
- **Symbol Search**: Search through thousands of symbols instantly
- **Statistics**: Real-time Flash and RAM usage percentages
- **Detail Modals**: Rich information panels for sections and symbols
- **Smart Filtering**: Filter symbols by section with visual indicators

## Usage

### Quick Start

1. Open `index.html` in your browser
2. The tool will automatically load `../../build/icarus_os.map`
3. Explore memory regions, sections, and symbols interactively

### Interactive Features

**Click on Memory Regions (Left Panel)**
- Filters the memory layout to show only that region's sections
- Highlights the selected region
- Shows filter indicator with clear button

**Click on Memory Blocks (Center Panel)**
- Opens detailed modal with section information
- Automatically filters symbol table to show only symbols in that section
- Shows symbol count and first 50 symbols
- Highlights the selected block

**Click on Symbols (Right Panel)**
- Opens detailed modal with symbol information
- Shows address, size, and containing section
- Displays section details and offset within section
- Color-coded badges for easy identification

**Search Symbols**
- Type in the search box to filter symbols in real-time
- Works with filtered views
- Instant results as you type

### Keyboard Shortcuts

- **Escape**: Close modal
- **Click outside modal**: Close modal

## Visual Design

Inspired by dark blue professional theme:
- Deep blue gradients (#0f3460, #16213e, #1a1a2e)
- Smooth animations and transitions
- Color-coded sections and badges
- Hover effects and visual feedback
- Professional typography and spacing

## Color Coding

- Red: Flash / .text sections
- Blue: RAM sections
- Purple: DTCM sections
- Teal: ITCM sections
- Orange: Stack sections
- Dark Gray: .bss sections
- Green: .data sections

## Technical Details

### Parser (`parser.js`)
- Parses GNU LD map file format
- Extracts memory regions, sections, and symbols
- Calculates usage statistics

### Visualizer (`visualizer.js`)
- Renders memory layout proportionally
- Color-codes sections by type
- Formats sizes in human-readable format

### App Controller (`app.js`)
- Handles file loading (auto and manual)
- Coordinates parser and visualizer
- Manages user interactions

## Browser Compatibility

Works in all modern browsers:
- Chrome/Edge (recommended)
- Firefox
- Safari

## Development

No build step required - pure vanilla JavaScript!

To modify:
1. Edit HTML structure in `index.html`
2. Adjust styling in `style.css`
3. Extend parser logic in `parser.js`
4. Add visualizations in `visualizer.js`
5. Update app flow in `app.js`

## Example Use Cases

1. **Debug Stack Overflow**: Check stack section sizes
2. **Optimize Flash Usage**: Identify large sections
3. **Find Symbols**: Quickly locate function addresses
4. **Memory Planning**: Visualize region utilization
5. **Linker Issues**: Spot overlapping sections

## Tips

- Use search to find specific symbols (e.g., "game", "task", "kernel")
- Click memory regions to focus on specific areas
- Hover over blocks for detailed tooltips
- Check statistics for quick overview

---

Built for ICARUS OS - STM32H750 Real-Time Operating System
