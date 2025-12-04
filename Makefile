clean:
	rm *.exe

game:
	gcc main.c -o game.exe \
		utils.c \
		renderer.c \
		deck_logic.c \
		creatures_logic.c \
		-lm -g\
		`pkg-config allegro-5 allegro_main-5 \
		allegro_font-5 \
		allegro_image-5 allegro_primitives-5 \
		 --libs --cflags`

game-win:
	gcc main.c -o game.exe \
		utils.c \
		renderer.c \
		deck_logic.c \
		creatures_logic.c \
		-lm -g\
		$(shell pkg-config allegro-5 allegro_main-5 \
		allegro_font-5 \
		allegro_image-5 allegro_primitives-5 \
		 --libs --cflags)
