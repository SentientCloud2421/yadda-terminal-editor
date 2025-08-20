# yadda-terminal-editor

Yadda, is a text editor that I wrote from scratch for a series of youtube videos. This current version is written in C++, and runs in the terminal. I can't confirm whether or not it runs crossplatform since it uses certain terminal escape sequences that are supported by XTerm.

## Features
A mostly working editor, with a crappy color scheme. I tried doing highlighting, and it did work, but it was super buggy, and only supported C++ code, so I pulled it out before I turned this into a git repo.

It uses 'h', 'j', 'k', and 'l', for navigation, and supports entering numbers to increase the distance. You can enter a number and press 'm' to move to an arbitrary line. ':' opens the command line, and you can use the 'w' command to save a file, 'q' to quit, and 'e' plus a filename to open a new file (closes the old one, so make sure you save first). You can also use home and end as normal. Delete and backspace work as usual.

'p' pastes the current clipboard using OSC52. 'y' starts a selection, that you can use to copy text, and 'd' starts a selection to copy and delete text. You must press enter to confirm the selection, backspace to delete it, or you can press escape to cancel the selection.

This editor uses a gap buffer, and stores line numbers.

## Issues
I'm not supporting this editor beyond what I need it to do, so no feature requests or bug reports will be heeded. This is only here so that viewers can find the code I write for videos. If you want to turn this piece of junk into a good editor, first of all, why, but second, you are welcome to do so.

## Contributions
I'm not taking any contributions for this editor. This isn't meant to be used by people who actually want to get stuff done, it's merely a stepping stone to something greater.
