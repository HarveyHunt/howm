#ifndef COMMAND_H
#define COMMAND_H

enum teleport_locations { TOP_LEFT, TOP_CENTER, TOP_RIGHT, CENTER, BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT };
enum arg_types {TYPE_IGNORE, TYPE_INT, TYPE_CMD};

typedef struct {
	char *name; /**< The function's name. */
	void (*func)(const Arg *); /**< The function to be called when a command
				     comes in from the socket. */
	void (*operator)(const unsigned int type, const int cnt); /**< The
			operator to be called when a command comes in from
			the socket. */
	int argc; /**< The amount of args this command expects. */
	int arg_type; /**< The argument's type for commands that use the union Arg. */
} Command;

/**
 * @brief Represents the last command (and its arguments) or the last
 * combination of operator, count and motion (ocm).
 */
struct replay_state {
	void (*last_op)(const unsigned int type, int cnt); /**< The last operator to be called. */
	void (*last_cmd)(const Arg *arg); /**< The last command to be called. */
	const Arg *last_arg; /**< The last argument, passed to the last command. */
	unsigned int last_type; /**< The value determine by the last motion
				(workspace, client etc).*/
	int last_cnt; /**< The last count passed to the last operator function. */
};

/**
 * @brief Represents a button.
 *
 * Allows the mapping of a button to a function, as is done with the Key struct
 * for keys.
 */
typedef struct {
	int mod; /**< The mask of the modifiers pressed.  */
	short int button; /**< The button that was pressed. */
	void (*func)(const Arg *); /**< The function to be called when the
					* button is pressed. */
	const Arg arg; /**< The argument passed to the above function. */
} Button;

/**
 * @brief Represents a key.
 *
 * Holds information relative to a key, such as keysym and the mode during
 * which the keypress can be seen as valid.
 */
typedef struct {
	int mod; /**< The mask of the modifiers pressed. */
	unsigned int mode; /**< The mode within which this keypress is valid. */
	xcb_keysym_t sym;  /**< The keysym of the pressed key. */
	void (*func)(const Arg *); /**< The function to be called when this key is pressed. */
	const Arg arg; /**< The argument passed to the above function. */
} Key;

/**
 * @brief Represents an argument.
 *
 * Used to hold data that is sent as a parameter to a function when called as a
 * result of a keypress.
 */
typedef union {
	const char * const * const cmd; /**< Represents a command that will be called by a shell.  */
	int i; /**< Usually used for specifying workspaces or clients. */
} Arg;

static struct replay_state rep_state;
void (*operator_func)(const unsigned int type, int cnt);

static void teleport_client(const Arg *arg);
void save_last_ocm(void (*op) (const unsigned int, int), const unsigned int type, int cnt);
void save_last_cmd(void (*cmd)(const Arg *), const Arg *arg);
static void move_current_down(const Arg *arg);
static void move_current_up(const Arg *arg);
static void focus_next_client(const Arg *arg);
static void focus_prev_client(const Arg *arg);
static void current_to_ws(const Arg *arg);
static void toggle_float(const Arg *arg);
static void resize_float_width(const Arg *arg);
static void resize_float_height(const Arg *arg);
static void move_float_y(const Arg *arg);
static void move_float_x(const Arg *arg);
static void toggle_fullscreen(const Arg *arg);
static void focus_urgent(const Arg *arg);
static void send_to_scratchpad(const Arg *arg);
static void get_from_scratchpad(const Arg *arg);
static void make_master(const Arg *arg);
static void toggle_bar(const Arg *arg);
static void resize_master(const Arg *arg);
static void focus_next_ws(const Arg *arg);
static void focus_prev_ws(const Arg *arg);
static void focus_last_ws(const Arg *arg);
static void change_ws(const Arg *arg);
static void change_mode(const Arg *arg);

#endif
