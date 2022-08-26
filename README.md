# Shell_Simulator

- Simulated shell built with C for Introduction to Computer Systems (CS 154)

## Features

- Can execute builtin shell commands:

  > ls, cd, echo

- Chaining and redirection

  > ls ; cd ; ls -la > output.txt

- Custom pipe to pre-append output to existing files

  > ls -la >+ output.txt

- Batch mode for large job queue processing

  > ./myshell large_input.txt

## How to Run

Run make to create binary and ./myshell to enter interactive mode.
