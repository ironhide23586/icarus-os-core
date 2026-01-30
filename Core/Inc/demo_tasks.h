/**
 * @file    demo_tasks.h
 * @brief   ICARUS OS Demo Task Registration Interface
 * @version 0.1.0
 *
 * @details Provides initialization for demonstration tasks that showcase
 *          ICARUS kernel features including semaphores and message pipes.
 *
 * @par Demo Configurations:
 *      - Semaphore producer/consumer with visual feedback
 *      - Single Producer → Single Consumer (SS)
 *      - Single Producer → Multiple Consumers (SM)
 *      - Multiple Producers → Single Consumer (MS)
 *      - Multiple Producers → Multiple Consumers (MM)
 *
 * @see     docs/do178c/design/SDD.md Section 8 - Demo Tasks
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef DEMO_TASKS_H_
#define DEMO_TASKS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * DEMO TASK API
 * ========================================================================= */

/**
 * @brief   Initialize and register all demo tasks
 *
 * @details Initializes IPC primitives and registers demonstration tasks:
 *          - Semaphore producer/consumer pair
 *          - Reference task (timing baseline)
 *          - Message queue demos (SS, SM, MS, MM configurations)
 *
 * @pre     os_init() must have been called
 * @post    Demo tasks registered and ready for scheduling
 *
 * @par Registered Tasks:
 *      | Name      | Type     | Description                    |
 *      |-----------|----------|--------------------------------|
 *      | producer  | Producer | Semaphore feed demo            |
 *      | consumer  | Consumer | Semaphore consume demo         |
 *      | reference | Utility  | Timing reference               |
 *      | ss_prod   | Producer | Single→Single pipe producer    |
 *      | ss_cons   | Consumer | Single→Single pipe consumer    |
 *      | sm_prod   | Producer | Single→Multi pipe producer     |
 *      | sm_con1/2 | Consumer | Single→Multi pipe consumers    |
 *      | ms_prd1/2 | Producer | Multi→Single pipe producers    |
 *      | ms_cons   | Consumer | Multi→Single pipe consumer     |
 *      | mm_prd1/2 | Producer | Multi→Multi pipe producers     |
 *      | mm_con1/2 | Consumer | Multi→Multi pipe consumers     |
 */
void demo_tasks_init(void);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_TASKS_H_ */
