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

void kill_ws(workspace_t *ws);
int correct_ws(unsigned int ws);
void focus_next_ws(void);
workspace_t *offset_ws(workspace_t *ws, int offset);
void focus_prev_ws(void);
void focus_last_ws(void);
void change_ws(const workspace_t *ws);
uint32_t workspace_to_index(const workspace_t *ws);
workspace_t *index_to_workspace(const monitor_t *m, uint32_t index);
void add_ws(monitor_t *m);
void remove_ws(monitor_t *m, workspace_t *ws);

#endif
