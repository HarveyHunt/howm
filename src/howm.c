#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <xcb/xcb_ewmh.h>

#include "handler.h"
#include "helper.h"
#include "howm.h"
#include "ipc.h"
#include "monitor.h"
#include "scratchpad.h"
#include "xcb_help.h"
#include "workspace.h"

/**
 * @file howm.c
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief The glue that holds howm together. This file houses the main event
 * loop as well as setup and cleanup.
 */

/*
 *┌────────────┐
 *│╻ ╻┏━┓╻ ╻┏┳┓│
 *│┣━┫┃ ┃┃╻┃┃┃┃│
 *│╹ ╹┗━┛┗┻┛╹ ╹│
 *└────────────┘
*/

static void setup(void);
static void cleanup(void);
static void exec_config(char *conf_path);

struct config conf = {
	.focus_mouse = false,
	.focus_mouse_click = true,
	.follow_move = true,
	.border_px = 2,
	.border_focus = 0,
	.border_unfocus = 0,
	.border_prev_focus = 0,
	.border_urgent = 0,
	.bar_bottom = true,
	.bar_height = 20,
	.op_gap_size = 4,
	.center_floating = true,
	.zoom_gap = true,
	.float_spawn_width = 500,
	.float_spawn_height = 500,
	.delete_register_size = 5,
	.scratchpad_height = 500,
	.scratchpad_width = 500,
};

bool running = true;
xcb_connection_t *dpy = NULL;
xcb_screen_t *screen = NULL;
xcb_ewmh_connection_t *ewmh = NULL;
const char *WM_ATOM_NAMES[] = { "WM_DELETE_WINDOW", "WM_PROTOCOLS" };
xcb_atom_t wm_atoms[LENGTH(WM_ATOM_NAMES)];

int retval = EXIT_FAILURE;
uint32_t border_focus = 0;
uint32_t border_unfocus = 0;
uint32_t border_prev_focus = 0;
uint32_t border_urgent = 0;
uint16_t screen_height = 0;
uint16_t screen_width = 0;
int cur_state = OPERATOR_STATE;
unsigned int mon_cnt = 0;
unsigned int workspace_cnt;

monitor_t *mon = NULL;
monitor_t *mon_head = NULL;
monitor_t *mon_tail = NULL;

/**
 * @brief Occurs when howm first starts.
 *
 * Workspaces are initialised, screen size is determined and atoms
 * are then grabbed.
 *
 * Atoms are gathered.
 */
static void setup(void)
{
	screen = xcb_setup_roots_iterator(xcb_get_setup(dpy)).data;
	if (!screen) {
		log_err("Can't acquire the default screen.");
		exit(EXIT_FAILURE);
	}

	screen_height = screen->height_in_pixels;
	screen_width = screen->width_in_pixels;

	get_atoms(WM_ATOM_NAMES, wm_atoms);
	setup_ewmh();
	scan_monitors();
	setup_ewmh_geom();

	xcb_prefetch_extension_data(dpy, &xcb_randr_id);

	conf.border_focus = get_colour(DEF_BORDER_FOCUS);
	conf.border_unfocus = get_colour(DEF_BORDER_UNFOCUS);
	conf.border_prev_focus = get_colour(DEF_BORDER_PREV_FOCUS);
	conf.border_urgent = get_colour(DEF_BORDER_URGENT);
	stack_init(&del_reg);

	howm_info();
}

/**
 * @brief The code that glues howm together...
 */
int main(int argc, char *argv[])
{
	fd_set descs;
	int sock_fd, dpy_fd, cmd_fd, ret;
	ssize_t n;
	xcb_generic_event_t *ev;
	char ch;
	char conf_path[128] = {0};
	char *data = calloc(IPC_BUF_SIZE, sizeof(char));

	if (!data) {
		log_err("Can't allocate memory for socket buffer.");
		exit(EXIT_FAILURE);
	}

	conf_path[0] = '\0';

	while ((ch = getopt(argc, argv, "vhc:")) != -1) {
		switch (ch) {
		case 'c':
			snprintf(conf_path, sizeof(conf_path), "%s", optarg);
			break;
		case 'v':
			printf("%s\n", VERSION);
			exit(EXIT_SUCCESS);
		case 'h':
			printf("%s: %s", WM_NAME, "[-v|-h|-c CONFIG_PATH]\n");
			exit(EXIT_SUCCESS);
		}
	}

	if (conf_path[0] == '\0') {
		snprintf(conf_path, sizeof(conf_path), "%s/%s/%s/%s", getenv("HOME"),
							".config", WM_NAME, CONF_NAME);
		log_err("Using default config path: %s", conf_path);
	}

	dpy = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(dpy)) {
		log_err("Can't open X connection");
		exit(EXIT_FAILURE);
	}

	setup();
	sock_fd = ipc_init();
	check_other_wm();
	dpy_fd = xcb_get_file_descriptor(dpy);
	exec_config(conf_path);

	while (running) {
		if (!xcb_flush(dpy))
			log_err("Failed to flush X connection");

		FD_ZERO(&descs);
		FD_SET(dpy_fd, &descs);
		FD_SET(sock_fd, &descs);

		if (select(MAX_FD(dpy_fd, sock_fd), &descs, NULL, NULL, NULL) > 0) {
			if (FD_ISSET(sock_fd, &descs)) {
				cmd_fd = accept(sock_fd, NULL, 0);
				if (cmd_fd == -1) {
					log_err("Failed to accept connection");
					continue;
				}
				n = read(cmd_fd, data, IPC_BUF_SIZE - 1);
				if (n > 0) {
					data[n] = '\0';
					ret = ipc_process(data, n);
					if (write(cmd_fd, &ret, sizeof(int)) == -1)
						log_err("Unable to send response. errno: %d", errno);
				}
			}
			if (FD_ISSET(dpy_fd, &descs)) {
				while ((ev = xcb_poll_for_event(dpy)) != NULL) {
					if (ev)
						handle_event(ev);
					else
						log_debug("Unimplemented event: %d", ev->response_type & ~0x80);
					free(ev);
				}
			}
			if (xcb_connection_has_error(dpy)) {
				log_err("XCB connection encountered an error.");
				running = false;
			}
		}
	}

	cleanup();
	close(sock_fd);
	free(data);

	if (!running)
		return retval;
}

/**
 * @brief Print debug information about the current state of howm.
 *
 * This can be parsed by programs such as scripts that will pipe their input
 * into a status bar.
 */
void howm_info(void)
{
#if DEBUG_ENABLE
	const workspace_t *ws;

	for (ws = mon->ws_head; ws != NULL; ws = ws->next) {
		fprintf(stdout, "%d:%u:%d:%u:%u\n",  ws->layout,
			workspace_to_index(ws), cur_state,
			ws->client_cnt, monitor_to_index(mon));
	}
	fflush(stdout);
#else
	fprintf(stdout, "%d:%d:%d:%u:%u\n",  mon->ws->layout,
		workspace_to_index(mon->ws), cur_state,
		mon->ws->client_cnt, monitor_to_index(mon));
	fflush(stdout);
#endif
}

/**
 * @brief Cleanup howm's resources.
 *
 * Delete all of the windows that have been created, remove button
 * grabs and remove pointer focus.
 */
static void cleanup(void)
{
	log_warn("Cleaning up");

	while (mon)
		remove_monitor(mon);

	xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT, screen->root,
			XCB_CURRENT_TIME);
	xcb_ewmh_connection_wipe(ewmh);
	if (ewmh)
		free(ewmh);
	stack_free(&del_reg);
	ipc_cleanup();
	xcb_disconnect(dpy);
}

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

/**
 * @brief Execute the script located at conf_path in order to configure howm.
 *
 * @param conf_path The file path to the config file.
 */
static void exec_config(char *conf_path)
{
	if (fork())
		return;
	setsid();
	execl(conf_path, conf_path, NULL);
	log_err("Couldn't execute the configuration file %s", conf_path);
}

/**
 * @brief Quit howm and set the return value.
 *
 * @param exit_status The return value that howm will send.
 *
 * @ingroup commands
 */
void quit(const int exit_status)
{
	log_warn("Quitting");
	retval = exit_status;
	running = false;
}

/**
 * @brief Spawns a command.
 *
 * @ingroup commands
 */
void spawn(char *cmd[])
{
	if (fork())
		return;
	if (dpy)
		close(screen->root);
	setsid();
	log_info("Spawning command: %s", (char *)cmd[0]);
	execvp((char *)cmd[0], (char **)cmd);
	log_err("execvp of command: %s failed.", (char *)cmd[0]);
	exit(EXIT_FAILURE);
}
