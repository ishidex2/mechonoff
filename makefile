wasm:
	rm -r ./bin/wasm
	mkdir ./bin/wasm
	em++ -std=c++17 -O2 -s ALLOW_MEMORY_GROWTH=1 -s MAX_WEBGL_VERSION=2 -s MIN_WEBGL_VERSION=2 -s USE_LIBPNG=1 -s USE_SDL_MIXER=1 ./src/main.cpp --preload-file assets -o ./bin/wasm/pge.html --shell-file shell.html

windows:
	i686-w64-mingw32-g++ ./src/main.cpp -lSDL_mixer -luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -lstdc++fs -static -std=c++17 -o ./bin/demo.exe

run:
	clang++ ./src/main.cpp -lX11 -lGL -lpthread -lpng -lstdc++ -lm --std=c++17 -lSDL_mixer -o ./bin/demo
	./bin/demo

run_wasm:
	cd bin/wasm; php -S localhost:3030
