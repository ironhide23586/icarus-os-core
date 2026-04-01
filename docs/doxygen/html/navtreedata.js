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
  [ "ICARUS OS (Intelligent Cooperative Architecture for Real-time Unified Systems)", "index.html", [
    [ "ICARUS OS Core", "index.html#autotoc_md0", [
      [ "", "index.html#autotoc_md1", null ],
      [ "Highlights", "index.html#autotoc_md2", null ],
      [ "Terminal GUI", "index.html#autotoc_md4", [
        [ "GUI Layout Overview", "index.html#autotoc_md5", null ],
        [ "GUI Components Explained", "index.html#autotoc_md6", [
          [ "Header Section", "index.html#autotoc_md7", null ],
          [ "Heartbeat Banner", "index.html#autotoc_md8", null ],
          [ "Task Progress Bars", "index.html#autotoc_md9", null ],
          [ "Semaphore Vertical Bars", "index.html#autotoc_md10", null ],
          [ "Message History Panels", "index.html#autotoc_md11", null ],
          [ "Stress Test Statistics", "index.html#autotoc_md12", null ]
        ] ],
        [ "Color Coding", "index.html#autotoc_md13", null ]
      ] ],
      [ "Quick Start", "index.html#autotoc_md15", [
        [ "Prerequisites", "index.html#autotoc_md16", null ],
        [ "Building the Firmware", "index.html#autotoc_md17", null ],
        [ "Memory Layout", "index.html#autotoc_md18", null ],
        [ "Flashing the Firmware", "index.html#autotoc_md19", null ],
        [ "Connecting to the Terminal", "index.html#autotoc_md20", null ],
        [ "What You'll See", "index.html#autotoc_md21", null ]
      ] ],
      [ "Overview", "index.html#autotoc_md23", [
        [ "Vision", "index.html#autotoc_md24", null ],
        [ "Key Features", "index.html#autotoc_md25", [
          [ "Core Kernel", "index.html#autotoc_md26", null ],
          [ "Memory Protection (v0.1.0)", "index.html#autotoc_md27", null ],
          [ "ICARUS Runner Game", "index.html#autotoc_md28", null ],
          [ "Demo & Testing", "index.html#autotoc_md29", null ],
          [ "Development & Quality", "index.html#autotoc_md30", null ]
        ] ]
      ] ],
      [ "Architecture Overview", "index.html#autotoc_md32", [
        [ "System Architecture", "index.html#autotoc_md33", null ],
        [ "Directory Structure", "index.html#autotoc_md34", null ],
        [ "Task Execution Flow", "index.html#autotoc_md35", null ],
        [ "IPC Data Flow Example (Semaphore)", "index.html#autotoc_md36", null ]
      ] ],
      [ "Memory Protection Architecture", "index.html#autotoc_md38", [
        [ "Protection Layers", "index.html#autotoc_md39", [
          [ "DTCM Protection (Kernel Data Isolation)", "index.html#autotoc_md40", null ],
          [ "ITCM Protection (Code Integrity)", "index.html#autotoc_md41", null ],
          [ "Task Data Isolation (Cross-Task Protection)", "index.html#autotoc_md42", null ]
        ] ],
        [ "SVC Call Gate Architecture", "index.html#autotoc_md43", null ],
        [ "Fault Handling", "index.html#autotoc_md44", null ],
        [ "Performance Impact", "index.html#autotoc_md45", null ],
        [ "Attack Test Results", "index.html#autotoc_md46", null ]
      ] ],
      [ "Tools & Utilities", "index.html#autotoc_md48", [
        [ "Memory Map Visualizer", "index.html#autotoc_md49", null ],
        [ "Terminal Connection Script", "index.html#autotoc_md50", null ],
        [ "Fault Blink Decoder", "index.html#autotoc_md51", null ]
      ] ],
      [ "DO-178C Alignment", "index.html#autotoc_md53", [
        [ "Documentation Suite", "index.html#autotoc_md54", null ],
        [ "Compliance Status", "index.html#autotoc_md55", null ],
        [ "Running Verification", "index.html#autotoc_md56", null ]
      ] ],
      [ "Project Status & Roadmap", "index.html#autotoc_md58", [
        [ "Current Status (v0.1.0)", "index.html#autotoc_md59", null ],
        [ "Roadmap", "index.html#autotoc_md60", [
          [ "v0.2.0 (Q2 2026) - Priority Scheduling", "index.html#autotoc_md61", null ],
          [ "v0.3.0 (Q3 2026) - Enhanced IPC", "index.html#autotoc_md62", null ],
          [ "v0.4.0 (Q4 2026) - Portability", "index.html#autotoc_md63", null ],
          [ "v0.5.0 (Q1 2027) - Advanced Features", "index.html#autotoc_md64", null ],
          [ "v1.0.0 (Q2 2027) - Certification Ready", "index.html#autotoc_md65", null ],
          [ "v2.0.0 (2028+) - AI Integration", "index.html#autotoc_md66", null ]
        ] ],
        [ "Contributing", "index.html#autotoc_md67", null ]
      ] ],
      [ "Support ICARUS OS", "index.html#autotoc_md69", null ]
    ] ],
    [ "Software Design Description (SDD)", "md_docs_2do178c_2design_2SDD.html", [
      [ "Document Control", "md_docs_2do178c_2design_2SDD.html#autotoc_md73", null ],
      [ "Revision History", "md_docs_2do178c_2design_2SDD.html#autotoc_md75", null ],
      [ "Introduction", "md_docs_2do178c_2design_2SDD.html#autotoc_md77", [
        [ "1.1 Purpose", "md_docs_2do178c_2design_2SDD.html#autotoc_md78", null ],
        [ "1.2 Scope", "md_docs_2do178c_2design_2SDD.html#autotoc_md79", null ],
        [ "1.3 Design Principles", "md_docs_2do178c_2design_2SDD.html#autotoc_md80", null ]
      ] ],
      [ "System Architecture", "md_docs_2do178c_2design_2SDD.html#autotoc_md82", [
        [ "2.1 Architectural Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md83", null ],
        [ "2.2 Component Summary", "md_docs_2do178c_2design_2SDD.html#autotoc_md84", null ]
      ] ],
      [ "Component Design", "md_docs_2do178c_2design_2SDD.html#autotoc_md86", [
        [ "3.1 Scheduler Component", "md_docs_2do178c_2design_2SDD.html#autotoc_md87", [
          [ "3.1.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md88", null ],
          [ "3.1.2 Scheduling Algorithm", "md_docs_2do178c_2design_2SDD.html#autotoc_md89", null ],
          [ "3.1.3 Timing Characteristics", "md_docs_2do178c_2design_2SDD.html#autotoc_md90", null ],
          [ "3.1.4 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md91", null ]
        ] ],
        [ "3.2 Task Manager Component", "md_docs_2do178c_2design_2SDD.html#autotoc_md93", [
          [ "3.2.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md94", null ],
          [ "3.2.2 Task Control Block (TCB)", "md_docs_2do178c_2design_2SDD.html#autotoc_md95", null ],
          [ "3.2.3 Task States", "md_docs_2do178c_2design_2SDD.html#autotoc_md96", null ],
          [ "3.2.4 Stack Frame Layout", "md_docs_2do178c_2design_2SDD.html#autotoc_md97", null ],
          [ "3.2.5 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md98", null ]
        ] ],
        [ "3.3 Context Switch Component", "md_docs_2do178c_2design_2SDD.html#autotoc_md100", [
          [ "3.3.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md101", null ],
          [ "3.3.2 Context Switch Flow", "md_docs_2do178c_2design_2SDD.html#autotoc_md102", null ],
          [ "3.3.3 Assembly Implementation", "md_docs_2do178c_2design_2SDD.html#autotoc_md103", null ],
          [ "3.3.4 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md104", null ]
        ] ],
        [ "3.4 Critical Section Component", "md_docs_2do178c_2design_2SDD.html#autotoc_md106", [
          [ "3.4.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md107", null ],
          [ "3.4.2 Implementation", "md_docs_2do178c_2design_2SDD.html#autotoc_md108", null ],
          [ "3.4.3 Design Rationale", "md_docs_2do178c_2design_2SDD.html#autotoc_md109", null ],
          [ "3.4.4 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md110", null ]
        ] ],
        [ "3.5 Semaphore Component", "md_docs_2do178c_2design_2SDD.html#autotoc_md112", [
          [ "3.5.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md113", null ],
          [ "3.5.2 Data Structure", "md_docs_2do178c_2design_2SDD.html#autotoc_md114", null ],
          [ "3.5.3 Semaphore Operations", "md_docs_2do178c_2design_2SDD.html#autotoc_md115", null ],
          [ "3.5.4 Blocking Behavior", "md_docs_2do178c_2design_2SDD.html#autotoc_md116", null ],
          [ "3.5.5 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md117", null ]
        ] ],
        [ "3.6 Message Pipe Component", "md_docs_2do178c_2design_2SDD.html#autotoc_md119", [
          [ "3.6.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md120", null ],
          [ "3.6.2 Data Structure", "md_docs_2do178c_2design_2SDD.html#autotoc_md121", null ],
          [ "3.6.3 Pipe Operations", "md_docs_2do178c_2design_2SDD.html#autotoc_md122", null ],
          [ "3.6.4 Circular Buffer Layout", "md_docs_2do178c_2design_2SDD.html#autotoc_md123", null ],
          [ "3.6.5 Pipe Characteristics", "md_docs_2do178c_2design_2SDD.html#autotoc_md124", null ],
          [ "3.6.6 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md125", null ]
        ] ],
        [ "3.7 Print Buffer Component", "md_docs_2do178c_2design_2SDD.html#autotoc_md127", [
          [ "3.7.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md128", null ],
          [ "3.7.2 Data Structure", "md_docs_2do178c_2design_2SDD.html#autotoc_md129", null ],
          [ "3.7.3 Buffer Operations", "md_docs_2do178c_2design_2SDD.html#autotoc_md130", null ]
        ] ],
        [ "3.8 Mutex Component (Planned)", "md_docs_2do178c_2design_2SDD.html#autotoc_md132", [
          [ "3.8.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md133", null ],
          [ "3.8.2 Message Queue Design", "md_docs_2do178c_2design_2SDD.html#autotoc_md134", null ],
          [ "3.8.3 Queue Operations", "md_docs_2do178c_2design_2SDD.html#autotoc_md135", null ],
          [ "3.8.4 Mutex Design", "md_docs_2do178c_2design_2SDD.html#autotoc_md136", null ],
          [ "3.8.5 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md137", null ]
        ] ],
        [ "3.9 AI Runtime Component (Planned)", "md_docs_2do178c_2design_2SDD.html#autotoc_md139", [
          [ "3.9.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md140", null ],
          [ "3.9.2 Architecture", "md_docs_2do178c_2design_2SDD.html#autotoc_md141", null ],
          [ "3.9.3 Model Structure", "md_docs_2do178c_2design_2SDD.html#autotoc_md142", null ],
          [ "3.9.4 Operator Specifications", "md_docs_2do178c_2design_2SDD.html#autotoc_md143", null ],
          [ "3.9.5 Determinism Guarantees", "md_docs_2do178c_2design_2SDD.html#autotoc_md144", null ],
          [ "3.9.6 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md145", null ]
        ] ]
      ] ],
      [ "Memory Architecture", "md_docs_2do178c_2design_2SDD.html#autotoc_md147", [
        [ "4.1 Memory Map (STM32H750)", "md_docs_2do178c_2design_2SDD.html#autotoc_md148", null ],
        [ "4.2 MPU Protection Architecture", "md_docs_2do178c_2design_2SDD.html#autotoc_md149", [
          [ "4.2.1 Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md150", null ],
          [ "4.2.2 MPU Region Configuration", "md_docs_2do178c_2design_2SDD.html#autotoc_md151", null ],
          [ "4.2.3 Memory Region Details", "md_docs_2do178c_2design_2SDD.html#autotoc_md152", null ],
          [ "4.2.4 Privilege Separation Model", "md_docs_2do178c_2design_2SDD.html#autotoc_md153", null ],
          [ "4.2.5 SVC Call Gates", "md_docs_2do178c_2design_2SDD.html#autotoc_md154", null ],
          [ "4.2.6 Fault Handling", "md_docs_2do178c_2design_2SDD.html#autotoc_md155", null ],
          [ "4.2.7 Protection Verification", "md_docs_2do178c_2design_2SDD.html#autotoc_md156", null ]
        ] ],
        [ "4.3 Stack Allocation", "md_docs_2do178c_2design_2SDD.html#autotoc_md157", null ],
        [ "4.4 AI Memory Budget", "md_docs_2do178c_2design_2SDD.html#autotoc_md158", null ]
      ] ],
      [ "Interface Design", "md_docs_2do178c_2design_2SDD.html#autotoc_md160", [
        [ "5.1 Kernel API Summary", "md_docs_2do178c_2design_2SDD.html#autotoc_md161", null ],
        [ "5.2 BSP Interface Summary", "md_docs_2do178c_2design_2SDD.html#autotoc_md162", null ]
      ] ],
      [ "Design Traceability Matrix", "md_docs_2do178c_2design_2SDD.html#autotoc_md164", [
        [ "6.1 Requirements → Design Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md165", [
          [ "6.1.1 Kernel Requirements", "md_docs_2do178c_2design_2SDD.html#autotoc_md166", null ],
          [ "6.1.2 BSP Requirements", "md_docs_2do178c_2design_2SDD.html#autotoc_md167", null ],
          [ "6.1.3 AI Runtime Requirements", "md_docs_2do178c_2design_2SDD.html#autotoc_md168", null ]
        ] ],
        [ "6.2 Design → Code Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md169", null ],
        [ "6.3 Traceability Summary", "md_docs_2do178c_2design_2SDD.html#autotoc_md170", null ]
      ] ],
      [ "Design Verification", "md_docs_2do178c_2design_2SDD.html#autotoc_md172", [
        [ "7.1 Design Review Checklist", "md_docs_2do178c_2design_2SDD.html#autotoc_md173", null ],
        [ "7.2 Design Metrics", "md_docs_2do178c_2design_2SDD.html#autotoc_md174", null ]
      ] ]
    ] ],
    [ "Plan for Software Aspects of Certification (PSAC)", "md_docs_2do178c_2plans_2PSAC.html", [
      [ "Document Control", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md178", null ],
      [ "Revision History", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md180", null ],
      [ "Table of Contents", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md182", null ],
      [ "Introduction", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md184", [
        [ "1.1 Purpose", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md185", null ],
        [ "1.2 Scope", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md186", null ],
        [ "1.3 Applicable Documents", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md187", null ],
        [ "1.4 Reference Documents", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md188", null ]
      ] ],
      [ "System Overview", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md190", [
        [ "2.1 System Description", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md191", null ],
        [ "2.2 System Functions", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md192", null ],
        [ "2.3 System Architecture", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md193", null ],
        [ "2.4 Hardware Platform", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md194", null ]
      ] ],
      [ "Software Overview", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md196", [
        [ "3.1 Software Functions", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md197", [
          [ "3.1.1 Kernel Functions", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md198", null ],
          [ "3.1.2 BSP Functions", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md199", null ]
        ] ],
        [ "3.2 Software Architecture", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md200", null ],
        [ "3.3 Software Partitioning", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md201", null ],
        [ "3.4 Deactivated Code", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md202", null ]
      ] ],
      [ "Certification Considerations", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md204", [
        [ "4.1 Software Level", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md205", null ],
        [ "4.2 DO-178C Objectives", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md206", null ],
        [ "4.3 Structural Coverage", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md207", null ],
        [ "4.4 Software Tool Qualification", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md208", null ]
      ] ],
      [ "Software Development Environment", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md210", [
        [ "5.1 Development Hardware", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md211", null ],
        [ "5.2 Development Software", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md212", null ],
        [ "5.3 Test Environment", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md213", [
          [ "5.3.1 Host-Based Testing", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md214", null ],
          [ "5.3.2 Target-Based Testing", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md215", null ]
        ] ],
        [ "5.4 Configuration Management", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md216", null ]
      ] ],
      [ "Software Life Cycle", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md218", [
        [ "6.1 Life Cycle Model", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md219", null ],
        [ "6.2 Software Planning Process", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md220", null ],
        [ "6.3 Software Requirements Process", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md221", null ],
        [ "6.4 Software Design Process", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md222", null ],
        [ "6.5 Software Coding Process", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md223", null ],
        [ "6.6 Integration Process", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md224", null ],
        [ "6.7 Software Verification Process", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md225", null ]
      ] ],
      [ "Software Life Cycle Data", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md227", [
        [ "7.1 Data Items", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md228", null ],
        [ "7.2 Data Control Categories", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md229", null ],
        [ "7.3 Traceability", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md230", null ]
      ] ],
      [ "Schedule", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md232", [
        [ "8.1 Development Milestones", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md233", null ],
        [ "8.2 Certification Milestones", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md234", null ]
      ] ],
      [ "Additional Considerations", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md236", [
        [ "9.1 Previously Developed Software", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md237", null ],
        [ "9.2 User-Modifiable Software", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md238", null ],
        [ "9.3 Field-Loadable Software", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md239", null ],
        [ "9.4 Multiple-Version Dissimilar Software", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md240", null ],
        [ "9.5 Certification Authority Coordination", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md241", null ]
      ] ],
      [ "Appendix A: Acronyms", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md243", null ],
      [ "Appendix B: Document Cross-Reference", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md245", null ]
    ] ],
    [ "Software Configuration Management Plan (SCMP)", "md_docs_2do178c_2plans_2SCMP.html", [
      [ "Document Control", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md249", null ],
      [ "Revision History", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md251", null ],
      [ "Introduction", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md253", [
        [ "1.1 Purpose", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md254", null ],
        [ "1.2 Scope", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md255", null ]
      ] ],
      [ "Configuration Management Organization", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md257", [
        [ "2.1 Roles", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md258", null ],
        [ "2.2 Tools", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md259", null ]
      ] ],
      [ "Configuration Identification", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md261", [
        [ "3.1 Configuration Items", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md262", null ],
        [ "3.2 Naming Conventions", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md263", null ],
        [ "3.3 Version Numbering", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md264", null ]
      ] ],
      [ "Baseline Management", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md266", [
        [ "4.1 Baseline Types", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md267", null ],
        [ "4.2 Baseline Contents", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md268", null ]
      ] ],
      [ "Change Control", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md270", [
        [ "5.1 Change Process", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md271", null ],
        [ "5.2 Pull Request Requirements", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md272", null ],
        [ "5.3 Change Categories", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md273", null ],
        [ "5.4 Emergency Changes", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md274", null ]
      ] ],
      [ "Build Management", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md276", [
        [ "6.1 Build Environment", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md277", null ],
        [ "6.2 Build Targets", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md278", null ],
        [ "6.3 Build Reproducibility", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md279", null ]
      ] ],
      [ "Release Management", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md281", [
        [ "7.1 Release Process", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md282", null ],
        [ "7.2 Release Artifacts", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md283", null ],
        [ "7.3 Release Notes Template", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md284", null ]
      ] ],
      [ "Problem Reporting", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md286", [
        [ "8.1 Issue Tracking", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md287", null ],
        [ "8.2 Problem Report Template", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md288", null ]
      ] ],
      [ "Audit and Status Accounting", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md290", [
        [ "9.1 Configuration Status Reports", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md291", null ],
        [ "9.2 Audit Trail", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md292", null ],
        [ "9.3 Periodic Reviews", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md293", null ]
      ] ],
      [ "Supplier Control", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md295", [
        [ "10.1 Third-Party Components", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md296", null ],
        [ "10.2 Supplier Change Management", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md297", null ]
      ] ]
    ] ],
    [ "Software Development Plan (SDP)", "md_docs_2do178c_2plans_2SDP.html", [
      [ "Document Control", "md_docs_2do178c_2plans_2SDP.html#autotoc_md301", null ],
      [ "Revision History", "md_docs_2do178c_2plans_2SDP.html#autotoc_md303", null ],
      [ "Introduction", "md_docs_2do178c_2plans_2SDP.html#autotoc_md305", [
        [ "1.1 Purpose", "md_docs_2do178c_2plans_2SDP.html#autotoc_md306", null ],
        [ "1.2 Scope", "md_docs_2do178c_2plans_2SDP.html#autotoc_md307", null ],
        [ "1.3 ICARUS Vision", "md_docs_2do178c_2plans_2SDP.html#autotoc_md308", null ]
      ] ],
      [ "Development Organization", "md_docs_2do178c_2plans_2SDP.html#autotoc_md310", [
        [ "2.1 Roles and Responsibilities", "md_docs_2do178c_2plans_2SDP.html#autotoc_md311", null ],
        [ "2.2 Development Teams", "md_docs_2do178c_2plans_2SDP.html#autotoc_md312", null ]
      ] ],
      [ "Software Life Cycle", "md_docs_2do178c_2plans_2SDP.html#autotoc_md314", [
        [ "3.1 Development Model", "md_docs_2do178c_2plans_2SDP.html#autotoc_md315", null ],
        [ "3.2 Iteration Cycle", "md_docs_2do178c_2plans_2SDP.html#autotoc_md316", null ]
      ] ],
      [ "Development Standards", "md_docs_2do178c_2plans_2SDP.html#autotoc_md318", [
        [ "4.1 Coding Standards", "md_docs_2do178c_2plans_2SDP.html#autotoc_md319", null ],
        [ "4.2 Naming Conventions", "md_docs_2do178c_2plans_2SDP.html#autotoc_md320", null ],
        [ "4.3 File Organization", "md_docs_2do178c_2plans_2SDP.html#autotoc_md321", null ]
      ] ],
      [ "Target Hardware Platforms", "md_docs_2do178c_2plans_2SDP.html#autotoc_md323", [
        [ "5.1 Supported Platforms", "md_docs_2do178c_2plans_2SDP.html#autotoc_md324", null ],
        [ "5.2 Hardware Abstraction Strategy", "md_docs_2do178c_2plans_2SDP.html#autotoc_md325", null ],
        [ "5.3 Memory Map (STM32H7 Reference)", "md_docs_2do178c_2plans_2SDP.html#autotoc_md326", null ]
      ] ],
      [ "Development Tools", "md_docs_2do178c_2plans_2SDP.html#autotoc_md328", [
        [ "6.1 Toolchain", "md_docs_2do178c_2plans_2SDP.html#autotoc_md329", null ],
        [ "6.2 Verification Tools", "md_docs_2do178c_2plans_2SDP.html#autotoc_md330", null ],
        [ "6.3 AI Development Tools", "md_docs_2do178c_2plans_2SDP.html#autotoc_md331", null ],
        [ "6.4 CI/CD Pipeline", "md_docs_2do178c_2plans_2SDP.html#autotoc_md332", null ]
      ] ],
      [ "AI Runtime Architecture", "md_docs_2do178c_2plans_2SDP.html#autotoc_md334", [
        [ "7.1 Design Philosophy", "md_docs_2do178c_2plans_2SDP.html#autotoc_md335", null ],
        [ "7.2 AI Subsystem Components", "md_docs_2do178c_2plans_2SDP.html#autotoc_md336", null ],
        [ "7.3 Certifiable AI Constraints", "md_docs_2do178c_2plans_2SDP.html#autotoc_md337", null ],
        [ "7.4 AI Use Cases", "md_docs_2do178c_2plans_2SDP.html#autotoc_md338", null ],
        [ "7.5 AI Certification Strategy", "md_docs_2do178c_2plans_2SDP.html#autotoc_md339", null ]
      ] ],
      [ "Differentiation from Competitors", "md_docs_2do178c_2plans_2SDP.html#autotoc_md341", [
        [ "8.1 Competitive Landscape", "md_docs_2do178c_2plans_2SDP.html#autotoc_md342", null ],
        [ "8.2 ICARUS Unique Value", "md_docs_2do178c_2plans_2SDP.html#autotoc_md343", null ],
        [ "8.3 Target Markets", "md_docs_2do178c_2plans_2SDP.html#autotoc_md344", null ]
      ] ],
      [ "Review Process", "md_docs_2do178c_2plans_2SDP.html#autotoc_md346", [
        [ "9.1 Review Types", "md_docs_2do178c_2plans_2SDP.html#autotoc_md347", null ],
        [ "9.2 Code Review Checklist", "md_docs_2do178c_2plans_2SDP.html#autotoc_md348", null ],
        [ "9.3 Independence Requirements", "md_docs_2do178c_2plans_2SDP.html#autotoc_md349", null ]
      ] ],
      [ "Problem Reporting", "md_docs_2do178c_2plans_2SDP.html#autotoc_md351", [
        [ "10.1 Problem Report Categories", "md_docs_2do178c_2plans_2SDP.html#autotoc_md352", null ],
        [ "10.2 Problem Report Workflow", "md_docs_2do178c_2plans_2SDP.html#autotoc_md353", null ]
      ] ],
      [ "Appendix A: Coding Standard Deviations", "md_docs_2do178c_2plans_2SDP.html#autotoc_md355", null ]
    ] ],
    [ "Software Quality Assurance Plan (SQAP)", "md_docs_2do178c_2plans_2SQAP.html", [
      [ "Document Control", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md359", null ],
      [ "Revision History", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md361", null ],
      [ "Introduction", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md363", [
        [ "1.1 Purpose", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md364", null ],
        [ "1.2 Scope", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md365", null ],
        [ "1.3 QA Independence", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md366", null ]
      ] ],
      [ "Quality Assurance Organization", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md368", [
        [ "2.1 Roles and Responsibilities", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md369", null ],
        [ "2.2 Authority", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md370", null ]
      ] ],
      [ "Quality Assurance Activities", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md372", [
        [ "3.1 Activity Overview", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md373", null ],
        [ "3.2 Process Assurance", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md374", null ],
        [ "3.3 Product Assurance", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md375", null ]
      ] ],
      [ "Reviews and Audits", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md377", [
        [ "4.1 Review Types", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md378", null ],
        [ "4.2 Audit Types", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md379", null ],
        [ "4.3 Review Checklists", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md380", [
          [ "4.3.1 Requirements Review Checklist", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md381", null ],
          [ "4.3.2 Code Review Checklist", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md382", null ],
          [ "4.3.3 Test Review Checklist", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md383", null ]
        ] ]
      ] ],
      [ "Standards and Procedures", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md385", [
        [ "5.1 Applicable Standards", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md386", null ],
        [ "5.2 Coding Standards Summary", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md387", null ],
        [ "5.3 Documentation Standards", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md388", null ]
      ] ],
      [ "Non-Conformance Management", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md390", [
        [ "6.1 Non-Conformance Categories", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md391", null ],
        [ "6.2 Non-Conformance Process", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md392", null ],
        [ "6.3 Escalation Path", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md393", null ]
      ] ],
      [ "Metrics and Reporting", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md395", [
        [ "7.1 Quality Metrics", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md396", null ],
        [ "7.2 Quality Dashboard", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md397", null ],
        [ "7.3 Reporting Schedule", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md398", null ]
      ] ],
      [ "Training", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md400", [
        [ "8.1 Required Training", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md401", null ],
        [ "8.2 Training Records", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md402", null ]
      ] ],
      [ "Records Management", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md404", [
        [ "9.1 QA Records", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md405", null ],
        [ "9.2 Record Access", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md406", null ]
      ] ],
      [ "Continuous Improvement", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md408", [
        [ "10.1 Process Improvement", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md409", null ],
        [ "10.2 Lessons Learned", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md410", null ]
      ] ]
    ] ],
    [ "Software Verification Plan (SVP)", "md_docs_2do178c_2plans_2SVP.html", [
      [ "Document Control", "md_docs_2do178c_2plans_2SVP.html#autotoc_md414", null ],
      [ "Revision History", "md_docs_2do178c_2plans_2SVP.html#autotoc_md416", null ],
      [ "Introduction", "md_docs_2do178c_2plans_2SVP.html#autotoc_md418", [
        [ "1.1 Purpose", "md_docs_2do178c_2plans_2SVP.html#autotoc_md419", null ],
        [ "1.2 Scope", "md_docs_2do178c_2plans_2SVP.html#autotoc_md420", null ],
        [ "1.3 Verification Objectives", "md_docs_2do178c_2plans_2SVP.html#autotoc_md421", null ]
      ] ],
      [ "Verification Strategy", "md_docs_2do178c_2plans_2SVP.html#autotoc_md423", [
        [ "2.1 Multi-Level Verification", "md_docs_2do178c_2plans_2SVP.html#autotoc_md424", null ],
        [ "2.2 Verification Methods", "md_docs_2do178c_2plans_2SVP.html#autotoc_md425", null ]
      ] ],
      [ "Test Environment", "md_docs_2do178c_2plans_2SVP.html#autotoc_md427", [
        [ "3.1 Host-Based Testing", "md_docs_2do178c_2plans_2SVP.html#autotoc_md428", null ],
        [ "3.2 Target-Based Testing", "md_docs_2do178c_2plans_2SVP.html#autotoc_md429", null ],
        [ "3.3 Simulation Testing", "md_docs_2do178c_2plans_2SVP.html#autotoc_md430", null ],
        [ "3.4 AI Model Testing", "md_docs_2do178c_2plans_2SVP.html#autotoc_md431", null ]
      ] ],
      [ "Test Categories", "md_docs_2do178c_2plans_2SVP.html#autotoc_md433", [
        [ "4.1 Unit Tests", "md_docs_2do178c_2plans_2SVP.html#autotoc_md434", null ],
        [ "4.2 Integration Tests", "md_docs_2do178c_2plans_2SVP.html#autotoc_md435", null ],
        [ "4.3 System Tests", "md_docs_2do178c_2plans_2SVP.html#autotoc_md436", null ],
        [ "4.4 Robustness Tests", "md_docs_2do178c_2plans_2SVP.html#autotoc_md437", null ],
        [ "4.5 Memory Protection Tests", "md_docs_2do178c_2plans_2SVP.html#autotoc_md438", null ]
      ] ],
      [ "Structural Coverage", "md_docs_2do178c_2plans_2SVP.html#autotoc_md440", [
        [ "5.1 Coverage Requirements (DAL C)", "md_docs_2do178c_2plans_2SVP.html#autotoc_md441", null ],
        [ "5.2 Coverage Analysis Process", "md_docs_2do178c_2plans_2SVP.html#autotoc_md442", null ],
        [ "5.3 Coverage Exclusions", "md_docs_2do178c_2plans_2SVP.html#autotoc_md443", null ],
        [ "5.4 Current Coverage Status", "md_docs_2do178c_2plans_2SVP.html#autotoc_md444", null ]
      ] ],
      [ "Static Analysis", "md_docs_2do178c_2plans_2SVP.html#autotoc_md446", [
        [ "6.1 Tools and Configuration", "md_docs_2do178c_2plans_2SVP.html#autotoc_md447", null ],
        [ "6.2 MISRA Compliance", "md_docs_2do178c_2plans_2SVP.html#autotoc_md448", null ],
        [ "6.3 Complexity Metrics", "md_docs_2do178c_2plans_2SVP.html#autotoc_md449", null ]
      ] ],
      [ "AI Verification", "md_docs_2do178c_2plans_2SVP.html#autotoc_md451", [
        [ "7.1 AI-Specific Verification Challenges", "md_docs_2do178c_2plans_2SVP.html#autotoc_md452", null ],
        [ "7.2 AI Verification Methods", "md_docs_2do178c_2plans_2SVP.html#autotoc_md453", null ],
        [ "7.3 AI Test Cases", "md_docs_2do178c_2plans_2SVP.html#autotoc_md454", null ],
        [ "7.4 AI Certification Artifacts", "md_docs_2do178c_2plans_2SVP.html#autotoc_md455", null ]
      ] ],
      [ "Test Procedures", "md_docs_2do178c_2plans_2SVP.html#autotoc_md457", [
        [ "8.1 Unit Test Execution", "md_docs_2do178c_2plans_2SVP.html#autotoc_md458", null ],
        [ "8.2 Integration Test Execution", "md_docs_2do178c_2plans_2SVP.html#autotoc_md459", null ],
        [ "8.3 Coverage Collection", "md_docs_2do178c_2plans_2SVP.html#autotoc_md460", null ]
      ] ],
      [ "Verification Records", "md_docs_2do178c_2plans_2SVP.html#autotoc_md462", [
        [ "9.1 Test Results Format", "md_docs_2do178c_2plans_2SVP.html#autotoc_md463", null ],
        [ "9.2 Verification Matrix", "md_docs_2do178c_2plans_2SVP.html#autotoc_md464", null ]
      ] ],
      [ "Verification Schedule", "md_docs_2do178c_2plans_2SVP.html#autotoc_md466", null ]
    ] ],
    [ "Software Requirements Specification (SRS)", "md_docs_2do178c_2requirements_2SRS.html", [
      [ "Document Control", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md478", null ],
      [ "Revision History", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md480", null ],
      [ "Introduction", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md482", [
        [ "1.1 Purpose", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md483", null ],
        [ "1.2 Scope", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md484", null ],
        [ "1.3 Definitions", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md485", null ]
      ] ],
      [ "System Requirements Allocation", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md487", null ],
      [ "High-Level Requirements (HLR)", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md489", [
        [ "3.1 Kernel Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md490", [
          [ "3.1.1 Task Management", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md491", null ],
          [ "3.1.2 Scheduling", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md492", null ],
          [ "3.1.3 Timing Services", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md493", null ],
          [ "3.1.4 Critical Sections", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md494", null ],
          [ "3.1.5 Inter-Process Communication (IPC)", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md495", null ],
          [ "3.1.6 Memory Management", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md496", null ],
          [ "3.1.7 Fault Handling", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md497", null ]
        ] ],
        [ "3.2 BSP Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md499", [
          [ "3.2.1 Platform Abstraction", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md500", null ],
          [ "3.2.2 Peripheral Drivers", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md501", null ],
          [ "3.2.3 Timing Hardware", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md502", null ]
        ] ],
        [ "3.3 AI Runtime Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md504", [
          [ "3.3.1 Model Management", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md505", null ],
          [ "3.3.2 Inference Execution", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md506", null ],
          [ "3.3.3 Supported Operators", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md507", null ],
          [ "3.3.4 Safety Features", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md508", null ]
        ] ]
      ] ],
      [ "Performance Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md510", [
        [ "4.1 Timing Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md511", null ],
        [ "4.2 Resource Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md512", null ],
        [ "4.3 Scalability Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md513", null ]
      ] ],
      [ "Interface Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md515", [
        [ "5.1 Kernel API", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md516", null ],
        [ "5.2 AI Runtime API", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md517", null ],
        [ "5.3 BSP Interface", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md518", null ]
      ] ],
      [ "Safety Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md520", [
        [ "6.1 Fault Detection", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md521", null ],
        [ "6.2 Fault Response", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md522", null ]
      ] ],
      [ "Traceability", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md524", [
        [ "7.1 Requirements to Tests", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md525", null ],
        [ "7.2 Requirements Status Summary", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md526", null ]
      ] ],
      [ "", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md527", null ]
    ] ],
    [ "Structural Coverage Analysis Report", "md_docs_2do178c_2verification_2coverage__analysis.html", [
      [ "Purpose", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md529", null ],
      [ "Scope", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md530", null ],
      [ "Coverage Metrics", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md531", [
        [ "3.1 Summary", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md532", null ],
        [ "3.2 Per-file coverage (instrumented lines)", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md533", null ],
        [ "3.3 Partially covered or deactivated areas", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md534", null ]
      ] ],
      [ "Test Environment", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md535", [
        [ "4.1 Host test configuration", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md536", null ],
        [ "4.2 Commands", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md537", null ],
        [ "4.3 Test results", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md538", null ]
      ] ],
      [ "Coverage gap analysis", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md539", [
        [ "5.1 Justified exclusions", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md540", null ],
        [ "5.2 Certification follow-up", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md541", null ]
      ] ],
      [ "Recommendations", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md542", [
        [ "6.1 For DAL C certification", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md543", null ],
        [ "6.2 For DAL B certification", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md544", null ],
        [ "6.3 For DAL A certification", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md545", null ]
      ] ],
      [ "Approval", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md546", null ],
      [ "Revision history", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md547", null ]
    ] ],
    [ "Deactivated Code Analysis", "md_docs_2do178c_2verification_2deactivated__code.html", [
      [ "Purpose", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md549", null ],
      [ "Definitions", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md550", null ],
      [ "Deactivated Code Inventory", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md551", [
        [ "3.1 Fault Handlers (Dead Code)", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md552", [
          [ "3.1.1 Error_Handler", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md553", null ],
          [ "3.1.2 NMI_Handler", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md555", null ],
          [ "3.1.3 HardFault_Handler", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md557", null ],
          [ "3.1.4 MemManage_Handler", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md559", null ],
          [ "3.1.5 BusFault_Handler", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md561", null ],
          [ "3.1.6 UsageFault_Handler", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md563", null ]
        ] ],
        [ "3.2 Context Switch Handler (Target-Only Code)", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md565", [
          [ "3.2.1 PendSV_Handler", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md566", null ]
        ] ],
        [ "3.3 Infinite Loop Tasks (Target-Only Code)", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md568", [
          [ "3.3.1 os_idle_task", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md569", null ],
          [ "3.3.2 os_heartbeart_task", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md571", null ],
          [ "3.3.3 os_transmit_printf_task", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md573", null ]
        ] ],
        [ "3.4 Static Helper Functions", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md575", [
          [ "3.4.1 dequeue_print_buffer", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md576", null ]
        ] ]
      ] ],
      [ "Safety Analysis", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md578", [
        [ "4.1 Impact Assessment", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md579", null ],
        [ "4.2 Conclusion", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md580", null ]
      ] ],
      [ "Approval", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md581", null ],
      [ "Revision History", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md582", null ]
    ] ],
    [ "Test Traceability Matrix", "md_docs_2do178c_2verification_2test__traceability.html", [
      [ "Purpose", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md584", null ],
      [ "Traceability Overview", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md585", null ],
      [ "Functional Requirements Traceability", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md586", [
        [ "3.1 Kernel - Task Management", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md587", null ],
        [ "3.2 Kernel - Task Scheduling", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md588", null ],
        [ "3.3 Kernel - Critical Sections", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md589", null ],
        [ "3.4 Kernel - Semaphores", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md590", null ],
        [ "3.5 Kernel - Message Pipes", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md591", null ],
        [ "3.6 Kernel - Print Buffer", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md592", null ],
        [ "3.7 BSP - Display", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md593", null ],
        [ "3.8 BSP - LED Control", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md594", null ],
        [ "3.9 BSP - Platform I/O", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md595", null ],
        [ "3.10 BSP - Standard I/O", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md596", null ],
        [ "3.11 BSP - Interrupt Handlers", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md597", null ],
        [ "3.12 Kernel - SVC Dispatch", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md598", null ]
      ] ],
      [ "Test Case Summary", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md599", null ],
      [ "Untested Requirements", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md600", null ],
      [ "Approval", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md601", null ],
      [ "Revision History", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md602", null ]
    ] ],
    [ "Topics", "topics.html", "topics" ],
    [ "Data Structures", "annotated.html", [
      [ "Data Structures", "annotated.html", "annotated_dup" ],
      [ "Data Structure Index", "classes.html", null ],
      [ "Data Fields", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Variables", "functions_vars.html", "functions_vars" ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "Globals", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", "globals_func" ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", "globals_defs" ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"group__KERNEL__CONFIG.html#gab180b584d54e375e600e8bf2f1bc6fe7",
"group__LSM9DS1.html#gafe2e174fd81e544aa46fadb36c5bcfbb",
"group__LSM9DS1__Interfaces__Functions.html",
"kernel_8h.html#a0b2301472f186765d40f1736ea9098fd",
"md_docs_2do178c_2plans_2SDP.html#autotoc_md328",
"pipe_8h.html#a95076f8c0003dbc51657f2de6a360a87",
"structicarus__pipe__t.html#a78d6e7850505fdf8ba8e7263bf31c5aa",
"structlsm9ds1__int__src__m__t.html#aea82ff57ab6fbde1f2555e89428bd1b6",
"unionlsm9ds1__reg__t.html#a7294c62ce46c74a9af81a960c43bfdeb"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';