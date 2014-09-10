# Contributing

Contributions are always welcomed, so start sending some PRs!

## Getting Started

Clone the repository:

    git clone https://github.com/HarveyHunt/howm.git
  
Change to the develop branch:

    git checkout develop
    
You might find it useful to have the generated Doxygen docs handy. They are hosted on github
[here](http://harveyhunt.github.io/howm/). 

Alternatively, change into the newly cloned howm directory and run:

    doxygen
    
To have the documentation automatically generated locally.
  
Then it is time to hack away until your heart is content.

## Tools of the trade

It's important to have some tools to test howm with. I like to use [Xephyr](http://www.freedesktop.org/wiki/Software/Xephyr/) and [x11trace](http://xtrace.alioth.debian.org/).

Xephyr is an X server in a window and makes it easy to test howm without having to change to a different X server. I invoke Xephyr using the following command:

    Xephyr -ac -br -screen 1024x768 :1

Then I change into the directory that howm is in and run the following:

    DISPLAY=:1 gdb howm
    
x11trace is used for seeing the communication between an X server and its clients. This is useful when trying to track down bugs involving communication with X clients as well as implementing EWMH compliance. For normal development, it isn't necessary to use x11trace.

## Code Style

I try to follow the [Linux Kernel Guide](https://www.kernel.org/doc/Documentation/CodingStyle) as closely as sanely possible.

I run checkpatch.pl from the Linux Kernel against the code and strive for no errors- you should do the same.

Running a code linter, such as [Splint](http://www.splint.org/) is always a good idea- try to minimise errors.

The hard rules are as follows:

  * Use tabs for indentation (that are equivalent to 8 spaces in size).
  * use_of_underscores_for_functions_variables_etc
  * Code should be clear- clarity over conciseness
  * Doxygen documentation needs to be updated for new code
  * Document new features in the README

## What comes after code?

You have written an awesome new feature and want to get it merged straight away- have you forgotten something?

Documentation is an important part of any project, howm is no different. Document all new features, as has been done
in the README already.

Please don't change any version numbers anywhere within the project.

howm uses [Doxygen](http://www.stack.nl/~dimitri/doxygen/) for documentation of the code, keep this up to date as you go.

If you add new #defines to the config file, it might be a good idea to add some _Static_assert()s to correct users if they enter illegal values in the config file.

## Sending PRs

Once you have made some cool changes, push to github and send a PR.

Make sure that the PR is targeted at the develop branch, as packages are built from the master branch and that should be kept as stable as possible.
