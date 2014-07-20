#howm (Beta)

[![Build Status](https://travis-ci.org/HarveyHunt/howm.svg?branch=develop)](https://travis-ci.org/HarveyHunt/howm)
[![Flattr this git repo](http://api.flattr.com/button/flattr-badge-large.png)](https://flattr.com/submit/auto?user_id=harveyhunt&url=https://github.com/HarveyHunt/howm&title=howm&language=&tags=github&category=software) 

###A lightweight, tiling X11 window manager that mimics vi by offering operators, motions and modes.

Contents
=====
* [Contributing](#howm/CONTRIBUTING.md)
* [Configuration](#configuration)
* [Motions](#motions)
* [Counts](#counts)
* [Operators](#operators)

##Configuration

Configuration is done through the included config.h file.

Keys, operators, colours, motions and more are defined in this file.

It has a layout that will be recognisable to anyone that has used similarly small WMs.

Each option is described in detail below:

* **MODKEY**: An alias for a modifier, making it easier to change your modifier in your config. Possible values can be found [here](http://www.x.org/releases/X11R7.6/doc/libX11/specs/XKB/xkblib.html#changing_modifiers).

```
#define MODKEY Mod4Mask
```

* **COUNT_MOD**: An alias for a modifier that is to be used when indicating a [count](#Counts). Possible values can be found [here](http://www.x.org/releases/X11R7.6/doc/libX11/specs/XKB/xkblib.html#changing_modifiers).

```
#define COUNT_MOD Mod1Mask
```

* **OTHER_MOD**: An alias for a modifier that is to be used to indicate motions and operators. Possible values can be found [here](http://www.x.org/releases/X11R7.6/doc/libX11/specs/XKB/xkblib.html#changing_modifiers).

```
#define OTHER_MOD Mod1Mask
```

* **WORKSPACES**: The number of workspaces that you wish to have. The following is a brief description from [here](http://linux.about.com/library/gnome/blgnome2n4.htm):

>Workspaces allow you to manage which windows are on your screen. You can imagine  as being virtual screens, which you can switch between at any time. Every workspace contains the same desktop, the same panels, and the same menus. However, you can run different applications, and open different windows in each workspace. The applications in each workspace will remain there when you switch to other .

```
#define WORKSPACES 3
```

* **FOCUS_MOUSE**: When true, moving the mouse cursor into a different window will focus that window.

```
#define FOCUS_MOUSE true
```

* **FOCUS_MOUSE_CLICK**: When true, clicking with the mouse cursor on a different window will focus it. If FOCUS_MOUSE is true, this is unnecessary.

```
#define FOCUS_MOUSE_CLICK false
```

* **FOLLOW_SPAWN**: When true, focus will change to a new window when it is spawned.

```
#define FOLLOW_SPAWN false
```

* **GAP**: The size (in pixels) of the "useless gap" to place between windows.

```
#define GAP 4
```

* **OP_GAP_SIZE**: The size (in pixel) that the operators op_shrink_gap and op_grow_gap change the gap size by.

```
#define OP_GAP_SIZE 2
```

* **DEBUG_ENABLE**: When true, debugging information is sent to STDOUT.

```
#define DEBUG_ENABLE true
```

* **BORDER_PX**: The size of the border drawn around each window.

```
#define BORDER_PX 4
```

* **BORDER_FOCUS**: The colour (in the form #RRGGBB) that the border around the currently focused window is.

```
#define BORDER_FOCUS #FF00FF
```

* **BORDER_UNFOCUS**: The colour (in the form #RRGGBB) that the border around unfocused windows is.

```
#define BORDER_UNFOCUS #00FF00
```

* **BORDER_PREV_FOCUS**: The colour (in the form #RRGGBB) that the border around last focused windows is.

```
#define BORDER_PREV_FOCUS #0000FF
```

* **BAR_HEIGHT**: The height of a status bar that can be displayed at the bottom or top of the screen.

```
#define BAR_HEIGHT 20
```

* **BAR_BOTTOM**: When true, the space for a bar will be reserved at the bottom of the screen. When false, the space will be reserved at the top of the screen.

```
#define BAR_BOTTOM true
```

* **CENTER_FLOATING**: Whether a window that has just been chaned to floating should be centered or not.

```
#define CENTER_FLOATING true
```

* **ZOOM_GAP**: Whether a gap should be drawn around a window when howm is in zoom layout.

```
#define ZOOM_GAP true
```

* **LOG_LEVEL**: How much detail should be logged. A LOG_LEVEL of INFO will log
  everything, LOG_WARN will log warnings and errors and LOG_ERR will log only
  errors. LOG_NONE means nothing will be logged. Logging level severity is as
  follows:
    * LOG_INFO
    * LOG_WARN
    * LOG_ERR
    * LOG_NONE

```
#define LOG_LEVEL LOG_ERR
```

* **MASTER_RATIO**: The ratio of the master window's size in stack modes compared to the size of the stack area.
can be from 0 to 1.

```
#define MASTER_RATIO 0.7
```

* **DEFAULT_WORKSPACE**: The workspace that should be selected upon startup.

```
#define DEFAULT_WORKSPACE 1
```

##Motions

For a good primer on motions, vim's [documentation](http://vimdoc.sourceforge.net/htmldoc/motion.html) explains them well.

Operators and motions are combined so that an operation can be performed on multiple things, such as clients or workspaces. The current supported motions are as follows:

* **Workspace**: Perform an operation on one or more workspaces.

* **Client**: Perform an operation on one or more clients.

##Counts

Counts be applied to a motion, to make an operator affect multiple things. For example, you can add a 3 before a motion, meaning that the operator will affect 3 of the motions. The modifier that is used is defined by COUNT_MOD.

For example:

```
q2w
```

Will kill 2 workspaces (assuming the correct modifier keys are pressed and default keybindings are being used).
##Operators

Operators perform an action upon one or more targets (identified by motions).

Below are descriptions of each operator, the motions that they can perform an action upon and the mode that they work in (Note, all examples assume that the correct modifier keys have been pressed and use the default keymappings.):

* **op_kill**: An operator that kills an arbitrary number of targets.
Can be used on:
  * Clients
  * Workspaces

Used in mode:
  * Normal

```
q4c
```
The above command will kill 4 clients, closing the applications and removing removing them from the workspace.

```
qw
```
The above command will kill one workspace. This means that all clients on the current workspace will be killed.

* **op_move_down**: An operator that moves a group of targets down one.
Can be used on:
  * Clients
  * Workspaces

Used in mode:
  * Normal

```
j2c
```
The above command moves 2 clients down one place in the workspace's client list. If a client is moved past the last place, then it is wrapped around and placed at the start of the workspace's client list.

```
j3w
```
The above command moves the contents of 3 workspaces down one workspace. If a workspace is to be moved beyond the last workspace, it is wrapped to the first workspace.
