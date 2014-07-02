#howm

 ┌────────────┐
 │╻ ╻┏━┓╻ ╻┏┳┓│
 │┣━┫┃ ┃┃╻┃┃┃┃│
 │╹ ╹┗━┛┗┻┛╹ ╹│
 └────────────┘


[![Build Status](https://travis-ci.org/HarveyHunt/howm.svg?branch=develop)](https://travis-ci.org/HarveyHunt/howm)
[![Flattr this git repo](http://api.flattr.com/button/flattr-badge-large.png)](https://flattr.com/submit/auto?user_id=harveyhunt&url=https://github.com/HarveyHunt/howm&title=howm&language=&tags=github&category=software) 


A lightweight, tiling X11 window manager that mimics vi by offering operators, motions and modes.

##Configuration

Configuration is done through the included config.h file.

Keys, operators, colours, motions and more are defined in this file.

It has a layout that will be recognisable to anyone that has used similarly small WMs.

Each option is described in detail below:

* **MODKEY**: An alias for a modifier, making it easier to change your modifier in your config. Possible values can be found [here](http://www.x.org/releases/X11R7.6/doc/libX11/specs/XKB/xkblib.html#changing_modifiers).

```
MODKEY = Mod4Mask
```

* **COUNT_MOD**: An alias for a modifier that is to be used when indicating a count. Possible values can be found [here](http://www.x.org/releases/X11R7.6/doc/libX11/specs/XKB/xkblib.html#changing_modifiers).

```
COUNT_MOD = Mod1Mask
```

* **OTHER_MOD**: An alias for a modifier that is to be used to indicate motions and operators. Possible values can be found [here](http://www.x.org/releases/X11R7.6/doc/libX11/specs/XKB/xkblib.html#changing_modifiers).

```
OTHER_MOD = Mod1Mask
```

* **WORKSPACES**: The number of workspaces that you wish to have. The following is a brief description from [here](http://linux.about.com/library/gnome/blgnome2n4.htm):

> Workspaces allow you to manage which windows are on your screen. You can imagine workspaces as being virtual screens, which you can switch between at any time. Every workspace contains the same desktop, the same panels, and the same menus. However, you can run different applications, and open different windows in each workspace. The applications in each workspace will remain there when you switch to other workspaces.

```
WORKSPACES = 3
```

* **FOCUS_MOUSE**: When true, moving the mouse cursor into a different window will focus that window.

```
FOCUS_MOUSE = true
```

* **FOCUS_MOUSE_CLICK**: When true, clicking with the mouse cursor on a different window will focus it. If FOCUS_MOUSE is true, this is unnecessary.

```
FOCUS_MOUSE_CLICK = false
```

* **FOLLOW_SPAWN**: When true, focus will change to a new window when it is spawned.

```
FOLLOW_SPAWN = false
```

* **GAP**: The size (in pixels) of the "useless gap" to place between windows.

```
GAP = 4
```

* **DEBUG_ENABLE**: When true, debugging information is sent to STDOUT.

```
DEBUG_ENABLE = true
```

* **BORDER_PX**: The size of the border drawn around each window.

```
BORDER_PX = 4
```

* **BORDER_FOCUS**: The colour (in the form #RRGGBB) that the border around the currently focused window is.

```
BORDER_FOCUS = #FF00FF
```

* **BORDER_UNFOCUS**: The colour (in the form #RRGGBB) that the border around unfocused windows is.

```
BORDER_UNFOCUS = #00FF00
```

* **BAR_HEIGHT**: The height of a status bar that can be displayed at the bottom or top of the screen.

```
BAR_HEIGHT = 20
```

* **BAR_BOTTOM**: When true, the space for a bar will be reserved at the bottom of the screen. When false, the space will be reserved at the top of the screen.

```
BAR_BOTTOM = true
```

##Operators

Operators perform an action upon one or more targets (identified by motions).

Below are descriptions of each operator, the motions that they can perform an action upon and the mode that they work in (Note, all examples assume that the correct modifier keys have been pressed and use the default keymappings.):

* **op_kill**: An operator that kills an arbitrary number of targets.
Can be used on:
  * Clients
  * Workspaces
Modes:
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
Modes:
  * Normal

```
j2c
```
The above command moves 2 clients down one place in the workspace's client list. If a client is moved past the last place, then it is wrapped around and placed at the start of the workspace's client list.

```
j3w
```
The above command moves the contents of 3 workspaces down one workspace. If a workspace is to be moved beyond the last workspace, it is wrapped to the first workspace.
