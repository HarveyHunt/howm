#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <stddef.h>

/**
 * @file workspace.h
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief howm
 */

void kill_ws(const int ws);
int correct_ws(unsigned int ws);
void focus_next_ws(void);
void focus_prev_ws(void);
void focus_last_ws(void);
void change_ws(const int ws);

#endif

