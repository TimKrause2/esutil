CPPFLAGS=-ggdb `pkg-config --cflags freetype2` -fPIC
CFLAGS=-ggdb -fPIC

libutil.a:esShader.o esfont.o esfontd.o lgraph.o
	ar r libutil.a esShader.o esfont.o esfontd.o lgraph.o

esShader.o:esShader.c

esfont.o:esfont.cc

esfontd.o:esfontd.cc

lgraph.o:lgraph.cpp
