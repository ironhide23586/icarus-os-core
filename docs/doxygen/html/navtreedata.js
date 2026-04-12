/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "ICARUS OBC", "index.html", [
    [ "ICARUS OS Core", "index.html#autotoc_md0", [
      [ "", "index.html#autotoc_md1", null ],
      [ "Highlights", "index.html#autotoc_md2", null ],
      [ "Terminal GUI — ICARUS Runner (Default)", "index.html#autotoc_md4", [
        [ "Game Screen", "index.html#autotoc_md5", null ],
        [ "Enabling the IPC Dashboard / Stress Tests", "index.html#autotoc_md6", null ],
        [ "IPC Dashboard Layout (when ENABLE_DEMO_TASKS / ENABLE_STRESS_TEST = 1)", "index.html#autotoc_md7", null ],
        [ "Dashboard Components Explained", "index.html#autotoc_md8", [
          [ "Header Section", "index.html#autotoc_md9", null ],
          [ "Heartbeat Banner", "index.html#autotoc_md10", null ],
          [ "Task Progress Bars", "index.html#autotoc_md11", null ],
          [ "Semaphore Vertical Bars", "index.html#autotoc_md12", null ],
          [ "Message History Panels", "index.html#autotoc_md13", null ],
          [ "Stress Test Statistics", "index.html#autotoc_md14", null ]
        ] ],
        [ "Dashboard Color Coding", "index.html#autotoc_md15", null ]
      ] ],
      [ "Quick Start", "index.html#autotoc_md17", [
        [ "Prerequisites", "index.html#autotoc_md18", null ],
        [ "Building the Firmware", "index.html#autotoc_md19", null ],
        [ "Memory Layout", "index.html#autotoc_md20", null ],
        [ "Flashing the Firmware", "index.html#autotoc_md21", null ],
        [ "Connecting to the Terminal", "index.html#autotoc_md22", null ],
        [ "What You'll See", "index.html#autotoc_md23", null ]
      ] ],
      [ "Running on the WeAct STM32H750VBT6", "index.html#autotoc_md25", [
        [ "What You Need", "index.html#autotoc_md26", null ],
        [ "Board Layout Reference", "index.html#autotoc_md27", null ],
        [ "Step 1 — Install Toolchain", "index.html#autotoc_md28", null ],
        [ "Step 2 — Build the Firmware", "index.html#autotoc_md29", null ],
        [ "Step 3 — Enter DFU Mode", "index.html#autotoc_md30", null ],
        [ "Step 4 — Flash", "index.html#autotoc_md31", null ],
        [ "Step 5 — Connect to the Terminal", "index.html#autotoc_md32", null ],
        [ "Pin Map Summary", "index.html#autotoc_md33", null ],
        [ "Troubleshooting", "index.html#autotoc_md34", null ]
      ] ],
      [ "Overview", "index.html#autotoc_md36", [
        [ "Vision", "index.html#autotoc_md37", null ],
        [ "Key Features", "index.html#autotoc_md38", [
          [ "Core Kernel", "index.html#autotoc_md39", null ],
          [ "Shared Modules (v0.3.0+)", "index.html#autotoc_md40", null ],
          [ "Memory Protection (v0.2.0)", "index.html#autotoc_md41", null ],
          [ "ICARUS Runner Game", "index.html#autotoc_md42", null ],
          [ "Demo & Testing", "index.html#autotoc_md43", null ],
          [ "Development & Quality", "index.html#autotoc_md44", null ]
        ] ]
      ] ],
      [ "Architecture Overview", "index.html#autotoc_md46", [
        [ "System Architecture", "index.html#autotoc_md47", null ],
        [ "Directory Structure", "index.html#autotoc_md48", null ],
        [ "Task Execution Flow", "index.html#autotoc_md49", null ],
        [ "IPC Data Flow Example (Semaphore)", "index.html#autotoc_md50", null ]
      ] ],
      [ "Memory Protection Architecture", "index.html#autotoc_md52", [
        [ "Protection Layers", "index.html#autotoc_md53", [
          [ "DTCM Protection (Kernel Data Isolation)", "index.html#autotoc_md54", null ],
          [ "ITCM Protection (Code Integrity)", "index.html#autotoc_md55", null ],
          [ "Task Data Isolation (Cross-Task Protection)", "index.html#autotoc_md56", null ]
        ] ],
        [ "SVC Call Gate Architecture", "index.html#autotoc_md57", null ],
        [ "Fault Handling", "index.html#autotoc_md58", null ],
        [ "Performance Impact", "index.html#autotoc_md59", null ],
        [ "Attack Test Results", "index.html#autotoc_md60", null ]
      ] ],
      [ "Tools & Utilities", "index.html#autotoc_md62", [
        [ "Memory Map Visualizer", "index.html#autotoc_md63", null ],
        [ "Terminal Connection Script", "index.html#autotoc_md64", null ],
        [ "Fault Blink Decoder", "index.html#autotoc_md65", null ]
      ] ],
      [ "DO-178C Alignment", "index.html#autotoc_md67", [
        [ "Documentation Suite", "index.html#autotoc_md68", null ],
        [ "Compliance Status", "index.html#autotoc_md69", null ],
        [ "Running Verification", "index.html#autotoc_md70", null ]
      ] ],
      [ "Project Status & Roadmap", "index.html#autotoc_md72", [
        [ "Current Status (v0.2.0)", "index.html#autotoc_md73", null ],
        [ "Roadmap", "index.html#autotoc_md74", [
          [ "v0.2.0 (Q2 2026) - Priority Scheduling", "index.html#autotoc_md75", null ],
          [ "v0.3.0 (Q3 2026) - Enhanced IPC", "index.html#autotoc_md76", null ],
          [ "v0.4.0 (Q4 2026) - Portability", "index.html#autotoc_md77", null ],
          [ "v0.5.0 (Q1 2027) - Advanced Features", "index.html#autotoc_md78", null ],
          [ "v1.0.0 (Q2 2027) - Certification Ready", "index.html#autotoc_md79", null ],
          [ "v2.0.0 (2028+) - AI Integration", "index.html#autotoc_md80", null ]
        ] ],
        [ "Contributing", "index.html#autotoc_md81", null ]
      ] ],
      [ "Support ICARUS OS", "index.html#autotoc_md83", null ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"index.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';