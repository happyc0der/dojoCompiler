# What is it?

A compiler for a subset of C written in C! Outputs x86-64 assembly. Right now we are at lexing tokens. I am using AI to tutor me, this code is written by me, I am not even copying snippets by AI, I am merely taking hints.

# Requirements

gcc or clang, and Python3 for the test suites

# Build and test

use ```make``` and ```make test``` and clone the test suites here using this command into the root folder of the repository using ```git clone git clone https://github.com/nlsandler/writing-a-c-compiler-tests.git```

# Usage
mycc flags, if the exit code is nonzero something has gone wrong!

# Layout
src/ contains main files that I have written.
tests/ custom tests for my own sanity check.
build/ build artefacts
drills/ assignment style problems given to me by AI so that I can understand better.

