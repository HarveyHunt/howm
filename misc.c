#include "misc.h"

/**
 * @brief Converts a hexcode colour into an X11 colourmap pixel.
 *
 * @param colour A string of the format "#RRGGBB", that will be interpreted as
 * a colour code.
 *
 * @return An X11 colourmap pixel.
 */
uint32_t get_colour(char *colour)
{
	uint32_t pixel;
	uint16_t r, g, b;
	xcb_alloc_color_reply_t *rep;
	xcb_colormap_t map = screen->default_colormap;

	long int rgb = strtol(++colour, NULL, 16);

	r = ((rgb >> 16) & 0xFF) * 257;
	g = ((rgb >> 8) & 0xFF) * 257;
	b = (rgb & 0xFF) * 257;
	rep = xcb_alloc_color_reply(dpy, xcb_alloc_color(dpy, map,
				    r, g, b), NULL);
	if (!rep) {
		log_err("ERROR: Can't allocate the colour %s", colour);
		return 0;
	}
	pixel = rep->pixel;
	free(rep);
	return pixel;
}


