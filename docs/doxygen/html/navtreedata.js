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
    [ "ICARUS OS Core", "index.html", "index" ],
    [ "Software Design Description (SDD)", "md_docs_2do178c_2design_2SDD.html", [
      [ "Document Control", "md_docs_2do178c_2design_2SDD.html#autotoc_md34", null ],
      [ "Revision History", "md_docs_2do178c_2design_2SDD.html#autotoc_md36", null ],
      [ "Introduction", "md_docs_2do178c_2design_2SDD.html#autotoc_md38", [
        [ "1.1 Purpose", "md_docs_2do178c_2design_2SDD.html#autotoc_md39", null ],
        [ "1.2 Scope", "md_docs_2do178c_2design_2SDD.html#autotoc_md40", null ],
        [ "1.3 Design Principles", "md_docs_2do178c_2design_2SDD.html#autotoc_md41", null ]
      ] ],
      [ "System Architecture", "md_docs_2do178c_2design_2SDD.html#autotoc_md43", [
        [ "2.1 Architectural Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md44", null ],
        [ "2.2 Component Summary", "md_docs_2do178c_2design_2SDD.html#autotoc_md45", null ]
      ] ],
      [ "Component Design", "md_docs_2do178c_2design_2SDD.html#autotoc_md47", [
        [ "3.1 Scheduler Component", "md_docs_2do178c_2design_2SDD.html#autotoc_md48", [
          [ "3.1.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md49", null ],
          [ "3.1.2 Scheduling Algorithm", "md_docs_2do178c_2design_2SDD.html#autotoc_md50", null ],
          [ "3.1.3 Timing Characteristics", "md_docs_2do178c_2design_2SDD.html#autotoc_md51", null ],
          [ "3.1.4 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md52", null ]
        ] ],
        [ "3.2 Task Manager Component", "md_docs_2do178c_2design_2SDD.html#autotoc_md54", [
          [ "3.2.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md55", null ],
          [ "3.2.2 Task Control Block (TCB)", "md_docs_2do178c_2design_2SDD.html#autotoc_md56", null ],
          [ "3.2.3 Task States", "md_docs_2do178c_2design_2SDD.html#autotoc_md57", null ],
          [ "3.2.4 Stack Frame Layout", "md_docs_2do178c_2design_2SDD.html#autotoc_md58", null ],
          [ "3.2.5 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md59", null ]
        ] ],
        [ "3.3 Context Switch Component", "md_docs_2do178c_2design_2SDD.html#autotoc_md61", [
          [ "3.3.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md62", null ],
          [ "3.3.2 Context Switch Flow", "md_docs_2do178c_2design_2SDD.html#autotoc_md63", null ],
          [ "3.3.3 Assembly Implementation", "md_docs_2do178c_2design_2SDD.html#autotoc_md64", null ],
          [ "3.3.4 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md65", null ]
        ] ],
        [ "3.4 Critical Section Component", "md_docs_2do178c_2design_2SDD.html#autotoc_md67", [
          [ "3.4.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md68", null ],
          [ "3.4.2 Implementation", "md_docs_2do178c_2design_2SDD.html#autotoc_md69", null ],
          [ "3.4.3 Design Rationale", "md_docs_2do178c_2design_2SDD.html#autotoc_md70", null ],
          [ "3.4.4 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md71", null ]
        ] ],
        [ "3.5 Semaphore Component", "md_docs_2do178c_2design_2SDD.html#autotoc_md73", [
          [ "3.5.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md74", null ],
          [ "3.5.2 Data Structure", "md_docs_2do178c_2design_2SDD.html#autotoc_md75", null ],
          [ "3.5.3 Semaphore Operations", "md_docs_2do178c_2design_2SDD.html#autotoc_md76", null ],
          [ "3.5.4 Blocking Behavior", "md_docs_2do178c_2design_2SDD.html#autotoc_md77", null ],
          [ "3.5.5 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md78", null ]
        ] ],
        [ "3.6 Message Pipe Component", "md_docs_2do178c_2design_2SDD.html#autotoc_md80", [
          [ "3.6.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md81", null ],
          [ "3.6.2 Data Structure", "md_docs_2do178c_2design_2SDD.html#autotoc_md82", null ],
          [ "3.6.3 Pipe Operations", "md_docs_2do178c_2design_2SDD.html#autotoc_md83", null ],
          [ "3.6.4 Circular Buffer Layout", "md_docs_2do178c_2design_2SDD.html#autotoc_md84", null ],
          [ "3.6.5 Pipe Characteristics", "md_docs_2do178c_2design_2SDD.html#autotoc_md85", null ],
          [ "3.6.6 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md86", null ]
        ] ],
        [ "3.7 Print Buffer Component", "md_docs_2do178c_2design_2SDD.html#autotoc_md88", [
          [ "3.7.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md89", null ],
          [ "3.7.2 Data Structure", "md_docs_2do178c_2design_2SDD.html#autotoc_md90", null ],
          [ "3.7.3 Buffer Operations", "md_docs_2do178c_2design_2SDD.html#autotoc_md91", null ]
        ] ],
        [ "3.8 Mutex Component (Planned)", "md_docs_2do178c_2design_2SDD.html#autotoc_md93", [
          [ "3.8.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md94", null ],
          [ "3.8.2 Message Queue Design", "md_docs_2do178c_2design_2SDD.html#autotoc_md95", null ],
          [ "3.8.3 Queue Operations", "md_docs_2do178c_2design_2SDD.html#autotoc_md96", null ],
          [ "3.8.4 Mutex Design", "md_docs_2do178c_2design_2SDD.html#autotoc_md97", null ],
          [ "3.8.5 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md98", null ]
        ] ],
        [ "3.9 AI Runtime Component (Planned)", "md_docs_2do178c_2design_2SDD.html#autotoc_md100", [
          [ "3.9.1 Design Overview", "md_docs_2do178c_2design_2SDD.html#autotoc_md101", null ],
          [ "3.9.2 Architecture", "md_docs_2do178c_2design_2SDD.html#autotoc_md102", null ],
          [ "3.9.3 Model Structure", "md_docs_2do178c_2design_2SDD.html#autotoc_md103", null ],
          [ "3.9.4 Operator Specifications", "md_docs_2do178c_2design_2SDD.html#autotoc_md104", null ],
          [ "3.9.5 Determinism Guarantees", "md_docs_2do178c_2design_2SDD.html#autotoc_md105", null ],
          [ "3.9.6 Requirements Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md106", null ]
        ] ]
      ] ],
      [ "Memory Architecture", "md_docs_2do178c_2design_2SDD.html#autotoc_md108", [
        [ "4.1 Memory Map (STM32H750)", "md_docs_2do178c_2design_2SDD.html#autotoc_md109", null ],
        [ "4.2 Stack Allocation", "md_docs_2do178c_2design_2SDD.html#autotoc_md110", null ],
        [ "4.3 AI Memory Budget", "md_docs_2do178c_2design_2SDD.html#autotoc_md111", null ]
      ] ],
      [ "Interface Design", "md_docs_2do178c_2design_2SDD.html#autotoc_md113", [
        [ "5.1 Kernel API Summary", "md_docs_2do178c_2design_2SDD.html#autotoc_md114", null ],
        [ "5.2 BSP Interface Summary", "md_docs_2do178c_2design_2SDD.html#autotoc_md115", null ]
      ] ],
      [ "Design Traceability Matrix", "md_docs_2do178c_2design_2SDD.html#autotoc_md117", [
        [ "6.1 Requirements → Design Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md118", [
          [ "6.1.1 Kernel Requirements", "md_docs_2do178c_2design_2SDD.html#autotoc_md119", null ],
          [ "6.1.2 BSP Requirements", "md_docs_2do178c_2design_2SDD.html#autotoc_md120", null ],
          [ "6.1.3 AI Runtime Requirements", "md_docs_2do178c_2design_2SDD.html#autotoc_md121", null ]
        ] ],
        [ "6.2 Design → Code Traceability", "md_docs_2do178c_2design_2SDD.html#autotoc_md122", null ],
        [ "6.3 Traceability Summary", "md_docs_2do178c_2design_2SDD.html#autotoc_md123", null ]
      ] ],
      [ "Design Verification", "md_docs_2do178c_2design_2SDD.html#autotoc_md125", [
        [ "7.1 Design Review Checklist", "md_docs_2do178c_2design_2SDD.html#autotoc_md126", null ],
        [ "7.2 Design Metrics", "md_docs_2do178c_2design_2SDD.html#autotoc_md127", null ]
      ] ]
    ] ],
    [ "Plan for Software Aspects of Certification (PSAC)", "md_docs_2do178c_2plans_2PSAC.html", [
      [ "Document Control", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md131", null ],
      [ "Revision History", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md133", null ],
      [ "Table of Contents", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md135", null ],
      [ "Introduction", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md137", [
        [ "1.1 Purpose", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md138", null ],
        [ "1.2 Scope", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md139", null ],
        [ "1.3 Applicable Documents", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md140", null ],
        [ "1.4 Reference Documents", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md141", null ]
      ] ],
      [ "System Overview", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md143", [
        [ "2.1 System Description", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md144", null ],
        [ "2.2 System Functions", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md145", null ],
        [ "2.3 System Architecture", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md146", null ],
        [ "2.4 Hardware Platform", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md147", null ]
      ] ],
      [ "Software Overview", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md149", [
        [ "3.1 Software Functions", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md150", [
          [ "3.1.1 Kernel Functions", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md151", null ],
          [ "3.1.2 BSP Functions", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md152", null ]
        ] ],
        [ "3.2 Software Architecture", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md153", null ],
        [ "3.3 Software Partitioning", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md154", null ],
        [ "3.4 Deactivated Code", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md155", null ]
      ] ],
      [ "Certification Considerations", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md157", [
        [ "4.1 Software Level", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md158", null ],
        [ "4.2 DO-178C Objectives", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md159", null ],
        [ "4.3 Structural Coverage", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md160", null ],
        [ "4.4 Software Tool Qualification", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md161", null ]
      ] ],
      [ "Software Development Environment", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md163", [
        [ "5.1 Development Hardware", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md164", null ],
        [ "5.2 Development Software", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md165", null ],
        [ "5.3 Test Environment", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md166", [
          [ "5.3.1 Host-Based Testing", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md167", null ],
          [ "5.3.2 Target-Based Testing", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md168", null ]
        ] ],
        [ "5.4 Configuration Management", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md169", null ]
      ] ],
      [ "Software Life Cycle", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md171", [
        [ "6.1 Life Cycle Model", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md172", null ],
        [ "6.2 Software Planning Process", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md173", null ],
        [ "6.3 Software Requirements Process", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md174", null ],
        [ "6.4 Software Design Process", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md175", null ],
        [ "6.5 Software Coding Process", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md176", null ],
        [ "6.6 Integration Process", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md177", null ],
        [ "6.7 Software Verification Process", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md178", null ]
      ] ],
      [ "Software Life Cycle Data", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md180", [
        [ "7.1 Data Items", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md181", null ],
        [ "7.2 Data Control Categories", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md182", null ],
        [ "7.3 Traceability", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md183", null ]
      ] ],
      [ "Schedule", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md185", [
        [ "8.1 Development Milestones", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md186", null ],
        [ "8.2 Certification Milestones", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md187", null ]
      ] ],
      [ "Additional Considerations", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md189", [
        [ "9.1 Previously Developed Software", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md190", null ],
        [ "9.2 User-Modifiable Software", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md191", null ],
        [ "9.3 Field-Loadable Software", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md192", null ],
        [ "9.4 Multiple-Version Dissimilar Software", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md193", null ],
        [ "9.5 Certification Authority Coordination", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md194", null ]
      ] ],
      [ "Appendix A: Acronyms", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md196", null ],
      [ "Appendix B: Document Cross-Reference", "md_docs_2do178c_2plans_2PSAC.html#autotoc_md198", null ]
    ] ],
    [ "Software Configuration Management Plan (SCMP)", "md_docs_2do178c_2plans_2SCMP.html", [
      [ "Document Control", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md202", null ],
      [ "Revision History", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md204", null ],
      [ "Introduction", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md206", [
        [ "1.1 Purpose", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md207", null ],
        [ "1.2 Scope", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md208", null ]
      ] ],
      [ "Configuration Management Organization", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md210", [
        [ "2.1 Roles", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md211", null ],
        [ "2.2 Tools", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md212", null ]
      ] ],
      [ "Configuration Identification", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md214", [
        [ "3.1 Configuration Items", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md215", null ],
        [ "3.2 Naming Conventions", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md216", null ],
        [ "3.3 Version Numbering", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md217", null ]
      ] ],
      [ "Baseline Management", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md219", [
        [ "4.1 Baseline Types", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md220", null ],
        [ "4.2 Baseline Contents", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md221", null ]
      ] ],
      [ "Change Control", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md223", [
        [ "5.1 Change Process", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md224", null ],
        [ "5.2 Pull Request Requirements", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md225", null ],
        [ "5.3 Change Categories", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md226", null ],
        [ "5.4 Emergency Changes", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md227", null ]
      ] ],
      [ "Build Management", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md229", [
        [ "6.1 Build Environment", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md230", null ],
        [ "6.2 Build Targets", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md231", null ],
        [ "6.3 Build Reproducibility", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md232", null ]
      ] ],
      [ "Release Management", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md234", [
        [ "7.1 Release Process", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md235", null ],
        [ "7.2 Release Artifacts", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md236", null ],
        [ "7.3 Release Notes Template", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md237", null ]
      ] ],
      [ "Problem Reporting", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md239", [
        [ "8.1 Issue Tracking", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md240", null ],
        [ "8.2 Problem Report Template", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md241", null ]
      ] ],
      [ "Audit and Status Accounting", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md243", [
        [ "9.1 Configuration Status Reports", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md244", null ],
        [ "9.2 Audit Trail", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md245", null ],
        [ "9.3 Periodic Reviews", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md246", null ]
      ] ],
      [ "Supplier Control", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md248", [
        [ "10.1 Third-Party Components", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md249", null ],
        [ "10.2 Supplier Change Management", "md_docs_2do178c_2plans_2SCMP.html#autotoc_md250", null ]
      ] ]
    ] ],
    [ "Software Development Plan (SDP)", "md_docs_2do178c_2plans_2SDP.html", [
      [ "Document Control", "md_docs_2do178c_2plans_2SDP.html#autotoc_md254", null ],
      [ "Revision History", "md_docs_2do178c_2plans_2SDP.html#autotoc_md256", null ],
      [ "Introduction", "md_docs_2do178c_2plans_2SDP.html#autotoc_md258", [
        [ "1.1 Purpose", "md_docs_2do178c_2plans_2SDP.html#autotoc_md259", null ],
        [ "1.2 Scope", "md_docs_2do178c_2plans_2SDP.html#autotoc_md260", null ],
        [ "1.3 ICARUS Vision", "md_docs_2do178c_2plans_2SDP.html#autotoc_md261", null ]
      ] ],
      [ "Development Organization", "md_docs_2do178c_2plans_2SDP.html#autotoc_md263", [
        [ "2.1 Roles and Responsibilities", "md_docs_2do178c_2plans_2SDP.html#autotoc_md264", null ],
        [ "2.2 Development Teams", "md_docs_2do178c_2plans_2SDP.html#autotoc_md265", null ]
      ] ],
      [ "Software Life Cycle", "md_docs_2do178c_2plans_2SDP.html#autotoc_md267", [
        [ "3.1 Development Model", "md_docs_2do178c_2plans_2SDP.html#autotoc_md268", null ],
        [ "3.2 Iteration Cycle", "md_docs_2do178c_2plans_2SDP.html#autotoc_md269", null ]
      ] ],
      [ "Development Standards", "md_docs_2do178c_2plans_2SDP.html#autotoc_md271", [
        [ "4.1 Coding Standards", "md_docs_2do178c_2plans_2SDP.html#autotoc_md272", null ],
        [ "4.2 Naming Conventions", "md_docs_2do178c_2plans_2SDP.html#autotoc_md273", null ],
        [ "4.3 File Organization", "md_docs_2do178c_2plans_2SDP.html#autotoc_md274", null ]
      ] ],
      [ "Target Hardware Platforms", "md_docs_2do178c_2plans_2SDP.html#autotoc_md276", [
        [ "5.1 Supported Platforms", "md_docs_2do178c_2plans_2SDP.html#autotoc_md277", null ],
        [ "5.2 Hardware Abstraction Strategy", "md_docs_2do178c_2plans_2SDP.html#autotoc_md278", null ],
        [ "5.3 Memory Map (STM32H7 Reference)", "md_docs_2do178c_2plans_2SDP.html#autotoc_md279", null ]
      ] ],
      [ "Development Tools", "md_docs_2do178c_2plans_2SDP.html#autotoc_md281", [
        [ "6.1 Toolchain", "md_docs_2do178c_2plans_2SDP.html#autotoc_md282", null ],
        [ "6.2 Verification Tools", "md_docs_2do178c_2plans_2SDP.html#autotoc_md283", null ],
        [ "6.3 AI Development Tools", "md_docs_2do178c_2plans_2SDP.html#autotoc_md284", null ],
        [ "6.4 CI/CD Pipeline", "md_docs_2do178c_2plans_2SDP.html#autotoc_md285", null ]
      ] ],
      [ "AI Runtime Architecture", "md_docs_2do178c_2plans_2SDP.html#autotoc_md287", [
        [ "7.1 Design Philosophy", "md_docs_2do178c_2plans_2SDP.html#autotoc_md288", null ],
        [ "7.2 AI Subsystem Components", "md_docs_2do178c_2plans_2SDP.html#autotoc_md289", null ],
        [ "7.3 Certifiable AI Constraints", "md_docs_2do178c_2plans_2SDP.html#autotoc_md290", null ],
        [ "7.4 AI Use Cases", "md_docs_2do178c_2plans_2SDP.html#autotoc_md291", null ],
        [ "7.5 AI Certification Strategy", "md_docs_2do178c_2plans_2SDP.html#autotoc_md292", null ]
      ] ],
      [ "Differentiation from Competitors", "md_docs_2do178c_2plans_2SDP.html#autotoc_md294", [
        [ "8.1 Competitive Landscape", "md_docs_2do178c_2plans_2SDP.html#autotoc_md295", null ],
        [ "8.2 ICARUS Unique Value", "md_docs_2do178c_2plans_2SDP.html#autotoc_md296", null ],
        [ "8.3 Target Markets", "md_docs_2do178c_2plans_2SDP.html#autotoc_md297", null ]
      ] ],
      [ "Review Process", "md_docs_2do178c_2plans_2SDP.html#autotoc_md299", [
        [ "9.1 Review Types", "md_docs_2do178c_2plans_2SDP.html#autotoc_md300", null ],
        [ "9.2 Code Review Checklist", "md_docs_2do178c_2plans_2SDP.html#autotoc_md301", null ],
        [ "9.3 Independence Requirements", "md_docs_2do178c_2plans_2SDP.html#autotoc_md302", null ]
      ] ],
      [ "Problem Reporting", "md_docs_2do178c_2plans_2SDP.html#autotoc_md304", [
        [ "10.1 Problem Report Categories", "md_docs_2do178c_2plans_2SDP.html#autotoc_md305", null ],
        [ "10.2 Problem Report Workflow", "md_docs_2do178c_2plans_2SDP.html#autotoc_md306", null ]
      ] ],
      [ "Appendix A: Coding Standard Deviations", "md_docs_2do178c_2plans_2SDP.html#autotoc_md308", null ]
    ] ],
    [ "Software Quality Assurance Plan (SQAP)", "md_docs_2do178c_2plans_2SQAP.html", [
      [ "Document Control", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md312", null ],
      [ "Revision History", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md314", null ],
      [ "Introduction", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md316", [
        [ "1.1 Purpose", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md317", null ],
        [ "1.2 Scope", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md318", null ],
        [ "1.3 QA Independence", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md319", null ]
      ] ],
      [ "Quality Assurance Organization", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md321", [
        [ "2.1 Roles and Responsibilities", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md322", null ],
        [ "2.2 Authority", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md323", null ]
      ] ],
      [ "Quality Assurance Activities", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md325", [
        [ "3.1 Activity Overview", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md326", null ],
        [ "3.2 Process Assurance", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md327", null ],
        [ "3.3 Product Assurance", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md328", null ]
      ] ],
      [ "Reviews and Audits", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md330", [
        [ "4.1 Review Types", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md331", null ],
        [ "4.2 Audit Types", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md332", null ],
        [ "4.3 Review Checklists", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md333", [
          [ "4.3.1 Requirements Review Checklist", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md334", null ],
          [ "4.3.2 Code Review Checklist", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md335", null ],
          [ "4.3.3 Test Review Checklist", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md336", null ]
        ] ]
      ] ],
      [ "Standards and Procedures", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md338", [
        [ "5.1 Applicable Standards", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md339", null ],
        [ "5.2 Coding Standards Summary", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md340", null ],
        [ "5.3 Documentation Standards", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md341", null ]
      ] ],
      [ "Non-Conformance Management", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md343", [
        [ "6.1 Non-Conformance Categories", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md344", null ],
        [ "6.2 Non-Conformance Process", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md345", null ],
        [ "6.3 Escalation Path", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md346", null ]
      ] ],
      [ "Metrics and Reporting", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md348", [
        [ "7.1 Quality Metrics", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md349", null ],
        [ "7.2 Quality Dashboard", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md350", null ],
        [ "7.3 Reporting Schedule", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md351", null ]
      ] ],
      [ "Training", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md353", [
        [ "8.1 Required Training", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md354", null ],
        [ "8.2 Training Records", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md355", null ]
      ] ],
      [ "Records Management", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md357", [
        [ "9.1 QA Records", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md358", null ],
        [ "9.2 Record Access", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md359", null ]
      ] ],
      [ "Continuous Improvement", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md361", [
        [ "10.1 Process Improvement", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md362", null ],
        [ "10.2 Lessons Learned", "md_docs_2do178c_2plans_2SQAP.html#autotoc_md363", null ]
      ] ]
    ] ],
    [ "Software Verification Plan (SVP)", "md_docs_2do178c_2plans_2SVP.html", [
      [ "Document Control", "md_docs_2do178c_2plans_2SVP.html#autotoc_md367", null ],
      [ "Revision History", "md_docs_2do178c_2plans_2SVP.html#autotoc_md369", null ],
      [ "Introduction", "md_docs_2do178c_2plans_2SVP.html#autotoc_md371", [
        [ "1.1 Purpose", "md_docs_2do178c_2plans_2SVP.html#autotoc_md372", null ],
        [ "1.2 Scope", "md_docs_2do178c_2plans_2SVP.html#autotoc_md373", null ],
        [ "1.3 Verification Objectives", "md_docs_2do178c_2plans_2SVP.html#autotoc_md374", null ]
      ] ],
      [ "Verification Strategy", "md_docs_2do178c_2plans_2SVP.html#autotoc_md376", [
        [ "2.1 Multi-Level Verification", "md_docs_2do178c_2plans_2SVP.html#autotoc_md377", null ],
        [ "2.2 Verification Methods", "md_docs_2do178c_2plans_2SVP.html#autotoc_md378", null ]
      ] ],
      [ "Test Environment", "md_docs_2do178c_2plans_2SVP.html#autotoc_md380", [
        [ "3.1 Host-Based Testing", "md_docs_2do178c_2plans_2SVP.html#autotoc_md381", null ],
        [ "3.2 Target-Based Testing", "md_docs_2do178c_2plans_2SVP.html#autotoc_md382", null ],
        [ "3.3 Simulation Testing", "md_docs_2do178c_2plans_2SVP.html#autotoc_md383", null ],
        [ "3.4 AI Model Testing", "md_docs_2do178c_2plans_2SVP.html#autotoc_md384", null ]
      ] ],
      [ "Test Categories", "md_docs_2do178c_2plans_2SVP.html#autotoc_md386", [
        [ "4.1 Unit Tests", "md_docs_2do178c_2plans_2SVP.html#autotoc_md387", null ],
        [ "4.2 Integration Tests", "md_docs_2do178c_2plans_2SVP.html#autotoc_md388", null ],
        [ "4.3 System Tests", "md_docs_2do178c_2plans_2SVP.html#autotoc_md389", null ],
        [ "4.4 Robustness Tests", "md_docs_2do178c_2plans_2SVP.html#autotoc_md390", null ]
      ] ],
      [ "Structural Coverage", "md_docs_2do178c_2plans_2SVP.html#autotoc_md392", [
        [ "5.1 Coverage Requirements (DAL C)", "md_docs_2do178c_2plans_2SVP.html#autotoc_md393", null ],
        [ "5.2 Coverage Analysis Process", "md_docs_2do178c_2plans_2SVP.html#autotoc_md394", null ],
        [ "5.3 Coverage Exclusions", "md_docs_2do178c_2plans_2SVP.html#autotoc_md395", null ],
        [ "5.4 Current Coverage Status", "md_docs_2do178c_2plans_2SVP.html#autotoc_md396", null ]
      ] ],
      [ "Static Analysis", "md_docs_2do178c_2plans_2SVP.html#autotoc_md398", [
        [ "6.1 Tools and Configuration", "md_docs_2do178c_2plans_2SVP.html#autotoc_md399", null ],
        [ "6.2 MISRA Compliance", "md_docs_2do178c_2plans_2SVP.html#autotoc_md400", null ],
        [ "6.3 Complexity Metrics", "md_docs_2do178c_2plans_2SVP.html#autotoc_md401", null ]
      ] ],
      [ "AI Verification", "md_docs_2do178c_2plans_2SVP.html#autotoc_md403", [
        [ "7.1 AI-Specific Verification Challenges", "md_docs_2do178c_2plans_2SVP.html#autotoc_md404", null ],
        [ "7.2 AI Verification Methods", "md_docs_2do178c_2plans_2SVP.html#autotoc_md405", null ],
        [ "7.3 AI Test Cases", "md_docs_2do178c_2plans_2SVP.html#autotoc_md406", null ],
        [ "7.4 AI Certification Artifacts", "md_docs_2do178c_2plans_2SVP.html#autotoc_md407", null ]
      ] ],
      [ "Test Procedures", "md_docs_2do178c_2plans_2SVP.html#autotoc_md409", [
        [ "8.1 Unit Test Execution", "md_docs_2do178c_2plans_2SVP.html#autotoc_md410", null ],
        [ "8.2 Integration Test Execution", "md_docs_2do178c_2plans_2SVP.html#autotoc_md411", null ],
        [ "8.3 Coverage Collection", "md_docs_2do178c_2plans_2SVP.html#autotoc_md412", null ]
      ] ],
      [ "Verification Records", "md_docs_2do178c_2plans_2SVP.html#autotoc_md414", [
        [ "9.1 Test Results Format", "md_docs_2do178c_2plans_2SVP.html#autotoc_md415", null ],
        [ "9.2 Verification Matrix", "md_docs_2do178c_2plans_2SVP.html#autotoc_md416", null ]
      ] ],
      [ "Verification Schedule", "md_docs_2do178c_2plans_2SVP.html#autotoc_md418", null ]
    ] ],
    [ "Software Requirements Specification (SRS)", "md_docs_2do178c_2requirements_2SRS.html", [
      [ "Document Control", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md427", null ],
      [ "Revision History", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md429", null ],
      [ "Introduction", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md431", [
        [ "1.1 Purpose", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md432", null ],
        [ "1.2 Scope", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md433", null ],
        [ "1.3 Definitions", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md434", null ]
      ] ],
      [ "System Requirements Allocation", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md436", null ],
      [ "High-Level Requirements (HLR)", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md438", [
        [ "3.1 Kernel Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md439", [
          [ "3.1.1 Task Management", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md440", null ],
          [ "3.1.2 Scheduling", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md441", null ],
          [ "3.1.3 Timing Services", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md442", null ],
          [ "3.1.4 Critical Sections", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md443", null ],
          [ "3.1.5 Inter-Process Communication (IPC)", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md444", null ],
          [ "3.1.6 Memory Management", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md445", null ],
          [ "3.1.7 Fault Handling", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md446", null ]
        ] ],
        [ "3.2 BSP Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md448", [
          [ "3.2.1 Platform Abstraction", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md449", null ],
          [ "3.2.2 Peripheral Drivers", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md450", null ],
          [ "3.2.3 Timing Hardware", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md451", null ]
        ] ],
        [ "3.3 AI Runtime Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md453", [
          [ "3.3.1 Model Management", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md454", null ],
          [ "3.3.2 Inference Execution", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md455", null ],
          [ "3.3.3 Supported Operators", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md456", null ],
          [ "3.3.4 Safety Features", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md457", null ]
        ] ]
      ] ],
      [ "Performance Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md459", [
        [ "4.1 Timing Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md460", null ],
        [ "4.2 Resource Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md461", null ],
        [ "4.3 Scalability Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md462", null ]
      ] ],
      [ "Interface Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md464", [
        [ "5.1 Kernel API", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md465", null ],
        [ "5.2 AI Runtime API", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md466", null ],
        [ "5.3 BSP Interface", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md467", null ]
      ] ],
      [ "Safety Requirements", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md469", [
        [ "6.1 Fault Detection", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md470", null ],
        [ "6.2 Fault Response", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md471", null ]
      ] ],
      [ "Traceability", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md473", [
        [ "7.1 Requirements to Tests", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md474", null ],
        [ "7.2 Requirements Status Summary", "md_docs_2do178c_2requirements_2SRS.html#autotoc_md475", null ]
      ] ]
    ] ],
    [ "Structural Coverage Analysis Report", "md_docs_2do178c_2verification_2coverage__analysis.html", [
      [ "Purpose", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md478", null ],
      [ "Scope", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md479", null ],
      [ "Coverage Metrics", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md480", [
        [ "3.1 Summary", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md481", null ],
        [ "3.2 Per-File Coverage", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md482", null ],
        [ "3.3 Uncovered Functions", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md483", null ]
      ] ],
      [ "Test Environment", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md484", [
        [ "4.1 Host Test Configuration", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md485", null ],
        [ "4.2 Test Execution", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md486", null ],
        [ "4.3 Test Results", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md487", null ]
      ] ],
      [ "Coverage Gap Analysis", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md488", [
        [ "5.1 Justified Exclusions", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md489", null ],
        [ "5.2 Effective Coverage", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md490", null ]
      ] ],
      [ "Recommendations", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md491", [
        [ "6.1 For DAL C Certification", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md492", null ],
        [ "6.2 For DAL B Certification", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md493", null ],
        [ "6.3 For DAL A Certification", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md494", null ]
      ] ],
      [ "Approval", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md495", null ],
      [ "Revision History", "md_docs_2do178c_2verification_2coverage__analysis.html#autotoc_md496", null ]
    ] ],
    [ "Deactivated Code Analysis", "md_docs_2do178c_2verification_2deactivated__code.html", [
      [ "Purpose", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md498", null ],
      [ "Definitions", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md499", null ],
      [ "Deactivated Code Inventory", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md500", [
        [ "3.1 Fault Handlers (Dead Code)", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md501", [
          [ "3.1.1 Error_Handler", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md502", null ],
          [ "3.1.2 NMI_Handler", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md504", null ],
          [ "3.1.3 HardFault_Handler", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md506", null ],
          [ "3.1.4 MemManage_Handler", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md508", null ],
          [ "3.1.5 BusFault_Handler", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md510", null ],
          [ "3.1.6 UsageFault_Handler", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md512", null ]
        ] ],
        [ "3.2 Context Switch Handler (Target-Only Code)", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md514", [
          [ "3.2.1 PendSV_Handler", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md515", null ]
        ] ],
        [ "3.3 Infinite Loop Tasks (Target-Only Code)", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md517", [
          [ "3.3.1 os_idle_task", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md518", null ],
          [ "3.3.2 os_heartbeart_task", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md520", null ],
          [ "3.3.3 os_transmit_printf_task", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md522", null ]
        ] ],
        [ "3.4 Static Helper Functions", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md524", [
          [ "3.4.1 dequeue_print_buffer", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md525", null ]
        ] ]
      ] ],
      [ "Safety Analysis", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md527", [
        [ "4.1 Impact Assessment", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md528", null ],
        [ "4.2 Conclusion", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md529", null ]
      ] ],
      [ "Approval", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md530", null ],
      [ "Revision History", "md_docs_2do178c_2verification_2deactivated__code.html#autotoc_md531", null ]
    ] ],
    [ "Test Traceability Matrix", "md_docs_2do178c_2verification_2test__traceability.html", [
      [ "Purpose", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md533", null ],
      [ "Traceability Overview", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md534", null ],
      [ "Functional Requirements Traceability", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md535", [
        [ "3.1 Kernel - Task Management", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md536", null ],
        [ "3.2 Kernel - Task Scheduling", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md537", null ],
        [ "3.3 Kernel - Critical Sections", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md538", null ],
        [ "3.4 Kernel - Semaphores", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md539", null ],
        [ "3.5 Kernel - Message Pipes", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md540", null ],
        [ "3.6 Kernel - Print Buffer", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md541", null ],
        [ "3.7 BSP - Display", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md542", null ],
        [ "3.8 BSP - LED Control", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md543", null ],
        [ "3.9 BSP - Platform I/O", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md544", null ],
        [ "3.10 BSP - Standard I/O", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md545", null ],
        [ "3.11 BSP - Interrupt Handlers", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md546", null ]
      ] ],
      [ "Test Case Summary", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md547", null ],
      [ "Untested Requirements", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md548", null ],
      [ "Approval", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md549", null ],
      [ "Revision History", "md_docs_2do178c_2verification_2test__traceability.html#autotoc_md550", null ]
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
"globals_type.html",
"group__LSM9DS1.html#ga9ceba5413d9e0751848dccbf7de88ad0",
"group__LSM9DS1__.html#gadf6649d4d7fd5d67880f1acd537d09ef",
"icarus__task_8c.html#a3475de8a0fed26a55e0c5d78cd475558",
"md_docs_2do178c_2plans_2SCMP.html#autotoc_md214",
"md_docs_2do178c_2verification_2test__traceability.html#autotoc_md540",
"structlsm9ds1__ctrl__reg3__m__t.html#a7ddbd37d94064c80950744428e7f58c3",
"structlsm9ds1__xl__trshld__en__t.html#a443f0c7626790b3c17a0cba7fd6848b7"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';