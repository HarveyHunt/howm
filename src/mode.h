#ifndef MODE_H
#define MODE_H

/**
 * @file mode.h
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief howm
 */

enum modes { NORMAL, FOCUS, FLOATING, END_MODES };

void change_mode(const unsigned int mode);

#endif
