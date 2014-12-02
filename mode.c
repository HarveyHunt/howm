#include "helper.h"
#include "howm.h"
#include "mode.h"

/**
 * @brief Change the mode of howm.
 *
 * Modes should be thought of in the same way as they are in vi. Different
 * modes mean keypresses cause different actions.
 *
 * @param mode The mode to be selected.
 *
 * @ingroup commands
 */
void change_mode(const int mode)
{
	if (mode >= (int)END_MODES || mode == (int)cur_mode)
		return;
	cur_mode = mode;
	log_info("Changing to mode %d", cur_mode);
	howm_info();
}
