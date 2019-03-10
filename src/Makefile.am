EXTRA_DIST = AUTHORS \
	     COPYING \
	     NEWS \
	     README \
	     INSTALL \
	     TODO \
	     ChangeLog \
	     autogen.sh \
	     Makefile.am \
	     configure.ac

transform = s/_/-/g

bin_PROGRAMS = j-quiz

j_quiz_LDADD = $(GTK_LIBS) $(SQLITE_LIBS) $(GTHREAD_LIBS) $(GSTREAMER_LIBS) -L/usr/lib/64 -lm
j_quiz_CFLAGS = $(GTK_CFLAGS) $(SQLITE_CFLAGS) $(GTHREAD_CFLAGS) $(GSTREAMER_CFLAGS)
j_quiz_SOURCES = main.c finish_frame.c groupnumber_dialog.c control_frame.c quiz_frame.c quizpad.c message_frame.c tomoe-canvas.c tomoe-writing.c wagomu.c

