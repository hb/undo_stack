all: tst.c claws_mail_undo.c claws_mail_undo.h
	gcc -Wall -g tst.c claws_mail_undo.c -o tst `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`