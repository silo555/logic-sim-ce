NAME = LOGICCE
DESCRIPTION = "logic sim CE"
ICON = icon.png
COMPRESSED = NO
ARCHIVED = YES
CFLAGS = -Wall -Wextra -Werror -Oz
include $(shell cedev-config --makefile)
