/*
 * demo_tasks.h
 *
 *  Created on: Jan 29, 2026
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 */

#ifndef DEMO_TASKS_H_
#define DEMO_TASKS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and register all demo tasks
 * 
 * This function initializes semaphores, message pipes, and registers
 * all demonstration tasks including:
 * - Semaphore producer/consumer demo
 * - Reference task
 * - Message queue demos (SS, SM, MS, MM configurations)
 */
void demo_tasks_init(void);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_TASKS_H_ */
