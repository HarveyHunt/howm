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
void change_mode(const unsigned int mode)
{
	if (mode >= END_MODES || mode == cur_mode)
		return;
	cur_mode = mode;
	log_info("Changing to mode %u", cur_mode);
	howm_info();
}
