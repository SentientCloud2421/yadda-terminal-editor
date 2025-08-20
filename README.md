# yadda-editor

Yadda, is a text editor that I wrote from scratch for a series of youtube videos. This current version is written in C++, and runs in the terminal. I can't confirm whether or not it runs crossplatform since it uses certain terminal escape sequences that are supported by XTerm.

## Features
A mostly working editor, with a crappy color scheme. I tried doing highlighting, and it did work, but it was super buggy, and only supported C++ code, so I pull it out before I turned this into a git repo.

It uses H, J, K, and L, for navigation, but it doesn't have any other vim motions. You can't pick a line and go there. ':' opens the command line, and you can use the w command to save a file, Q to quit, and E plus a filename to open a new file (closes the old one, so make sure you save first). You can also use home and end as normal. Delete and backspace work as usual.

P pastes the current clipboard using OSC52. Y starts a selection, that you can use to copy text, (max of like 500 characters or so), and D starts a selection to delete text. You must press enter to confirm the selection, or you can press escape to cancel it.

This editor uses a gap buffer, and supports utf-8 characters.

## Issues
I'm not supporting this editor beyond what I need it to do, so no feature requests or bug reports will be heeded. This is only here so that viewers can find the code I write for videos. If you want to turn this piece of junk into a good editor, first of all, why, but second, you are welcome to do so.

## Contributions
I'm not taking any contributions for this editor. This isn't meant to be used by people who actually want to get stuff done, it's merely a stepping stone to something greater.
