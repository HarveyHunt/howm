# howm (Beta)

[![Build Status](https://travis-ci.org/HarveyHunt/howm.svg?branch=develop)](https://travis-ci.org/HarveyHunt/howm)
[![Flattr this git repo](http://api.flattr.com/button/flattr-badge-large.png)](https://flattr.com/submit/auto?user_id=harveyhunt&url=https://github.com/HarveyHunt/howm&title=howm&language=&tags=github&category=software)

### A lightweight, tiling X11 window manager that mimics vi by offering operators, motions and modes.

![](http://i.imgur.com/4sW6RlT.gif)
![](http://i.imgur.com/tyiZcLx.gif)

Howm is on the [AUR](https://aur.archlinux.org/), there are two packages for it:
* [howm-x11-git](https://aur.archlinux.org/packages/howm-x11-git/) is the bleeding edge package.
* [howm-x11](https://aur.archlinux.org/packages/howm-x11/) is the package based off of stable releases.

Come and join us on Freenode in the channel #howm

Contents
=====
* [Contributing](CONTRIBUTING.md)
* [Requirements](#requirements)
* [Installation](#installation)
* [Commandline Arguments](#commandline-arguments)
* [Configuration](#configuration)
* [Changing Socket Path](#changing-socket-path)
* [Keybinds](#keybinds)
* [Scratchpad](#scratchpad)
* [Motions](#motions)
* [Counts](#counts)
* [Operators](#operators)
* [Modes](#modes)
* [Code Documentation](#code-documentation)
* [Parsing Output](#parsing-output)

## Requirements

* [Cottage](https://www.github.com/HarveyHunt/cottage) is required for configuration and interacting with howm.
* [sxhkd](https://www.github.com/baskerville/sxhkd) is required for binding cottage commands to keypress.

## Installation
Howm is on the [AUR](https://aur.archlinux.org/), there are two packages for it:
* [howm-git](https://aur.archlinux.org/packages/howm-x11-git/) is the bleeding edge package.
* [howm-x11](https://aur.archlinux.org/packages/howm-x11/) is the package based off of stable releases.

If you can't use the AUR, then there may be a package available for your distro.

As a last resort install xcb's libraries and then, do the following:

```bash
git clone https://github.com/HarveyHunt/howm
cd howm
make
sudo make install
```

After howm is installed, you'll want to copy over some configuration files.

```bash
cp examples/sxhkdrc_* ~/.config/sxhkd
cp examples/howmrc ~/.config/howmrc
```

Then take a look at the example [xinitrc](examples/xinitrc) for ideas on how to start howm.

Be sure to install [cottage](https://github.com/HarveyHunt/cottage) and [sxhkd](https://github.com/baskerville/sxhkd).

## Commandline Arguments

* **-c**: The path that points to an executable howmrc file.
```
howm -c ~/.config/howm/howmrc
```

## Configuration

Configuration is done through the use of cottage. Any element [in this structure](http://harveyhunt.github.io/howm/structconfig.html) can be changed using cottage. The syntax is as follows:

```
cottage -c config_var value
```

For example:

```
cottage -c bar_height 20
```

To override howm's default values at startup, cottage commands can be placed in a shell script and then executed by howm. Take a look at the [example howmrc](examples/howmrc) for ideas.

Note: When configuring colours in ```howmrc```, enclose the colour in quotes, such as:

```
cottage -c border_focus "#343434"
```

## Changing Socket Path
By default, howm will attempt to create a socket at ```/tmp/howm```, this can be overwritten by setting the environment variable ```HOWM_SOCK```. For example:

```bash
export HOWM_SOCK=/tmp/howm_test
```

## Keybinds

Keybinds are now placed in multiple [sxhkd](https://github.com/baskerville/sxhkd) files.

A keypress is bound to a call to cottage in the following form:

```
cottage -f func_name <args>
```

All of the available functions can be found [here](http://harveyhunt.github.io/howm/group__commands.html).
Take a look at the [example sxhkdrcs](examples).

## Scratchpad

The scratchpad is a location to store a single client out of view. When requesting a client back from the scratchpad, it will float in the center of the screen. This is useful for keeping a terminal handy or hiding your music player- only displaying it when it is really needed.

The size of the scratchpad's client is defined by SCRATCHPAD_WIDTH and SCRATCHPAD_HEIGHT.

## Motions

For a good primer on motions, vim's [documentation](http://vimdoc.sourceforge.net/htmldoc/motion.html) explains them well.

**Please note: The modifier key that is OTHER_MOD needs to be held down whilst entering a motion.**

Operators and motions are combined so that an operation can be performed on multiple things, such as clients or workspaces. The current supported motions are as follows:

* **Workspace**: Perform an operation on one or more workspaces.

* **Client**: Perform an operation on one or more clients.

## Counts

Counts be applied to a motion, to make an operator affect multiple things. For example, you can add a 3 before a motion, meaning that the operator will affect 3 of the motions. The modifier that is used is defined by COUNT_MOD.

For example:

```
q2w
```

Will kill 2 workspaces (assuming the correct modifier keys are pressed and default keybindings are being used).

## Operators

Operators perform an action upon one or more targets (identified by motions).

Implementation details for all operators can be found [here](http://harveyhunt.github.io/howm/group__operators.html).

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
The above command will kill 4 clients, closing the applications and removing them from the workspace.

```
qw
```
The above command will kill one workspace. This means that all clients on the current workspace will be killed.

* **op_move_down**: An operator that moves a group of targets down one.
Can be used on:
  * Clients

  Used in mode:
    * Normal

```
j2c
```
The above command moves 2 clients down one place in the workspace's client list. If a client is moved past the last place, then it is wrapped around and placed at the start of the workspace's client list.

* **op_move_up**: This is the opposite of op_move_down, and instead is bound to k.

* **op_shrink_gaps**: An operator to shrink the size of the gaps around windows. The size is changed by the amount defined for OP\_GAP\_SIZE.
Can be used on:
  * Clients
  * Workspaces

  Used in mode:
    * Normal

```
g1w
```

The above command will shrink the gaps of all windows on the current workspace by OP\_GAP\_SIZE.

```
g4c
```

The above command will shrink the gaps of 4 clients on the current workspace by OP\_GAP\_SIZE.

* **op_grow_gaps**: This is the opposite of op\_shrink\_gaps and is bound to Shift + g.

* **op_focus_up**: Move the current focus up.
Can be used on:
  * Clients
  * Workspaces

  Used in mode:
    * Focus

```
j3c
```

The above command will move the current focus down 3 clients.

* **op_focus_down**: Performs the opposite of op\_focus\_up and is instead bound to j.

* **op_cut**: Cut a group of clients or workspaces and store them on the delete register stack.
Can be used on:
  * Clients
  * Workspaces
  
  Used in mode:
    * Normal

```
d2c
```

The above command will cut 2 clients and place them onto the delete register stack. One use of the cut operation takes up one place on the stack.


## Modes

A good primer on modes is available [here](http://vimdoc.sourceforge.net/htmldoc/intro.html#vim-modes-intro).

**Note**: Modes used to be implemented in howm itself, but are now in the sxhkd
configuration file.

In howm, modes are used to allow the same keys to be bound to multiple functions. Modes also help to logically separate what needs to be done to a window. The available modes are as follows:

* **Normal**: This mode is the one that you will spend most of your time in. It is used for executing commands and most of the operators are designed to work in this mode. This mode behaves similarly to how other WMs behave, but without focusing or dealing with floating windows.

* **Focus**: This mode is designed to be used to change the focus and locations of windows or workspaces.

* **Floating**: This mode is designed to deal with all things floating. Moving, resizing and teleporting floating windows are all available in this mode.

## Code Documentation

howm uses [doxygen](http://www.stack.nl/~dimitri/doxygen/) throughout the entire codebase. The generated documentation is [available here](https://harveyhunt.github.io/howm/).

## Parsing Output

When debug mode is disabled, howm outputs information about its current state and the current workspace whenever something changes (such as adding a new window). When debug mode is enabled, information is outputted for each workspace (placed on a new line).

The format for the output is as follows:

```
Layout:Workspace:State:NumberofClients:MonitorIndex
```

An example output can be seen below:

```
2:1:0:1:0
```

The information outputted at the same time as the example above, but with debugging mode turned on is shown below:

```
2:1:0:1:0
2:2:0:0:0
2:3:0:0:0
2:4:0:0:0
2:5:0:0:0
```
