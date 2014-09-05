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


## Sending PRs

Once you have made some cool changes, push to github and send a PR.

Make sure that the PR is targeted at the develop branch, as packages are built from the master branch and that should be kept as stable as possible.
