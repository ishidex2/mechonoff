#include <iostream>

#include <SDL/SDL_mixer.h>

#define DBG(msg) std::cout << "[Sounder] " << msg << std::endl
#define DBG2(msg, msg2) std::cout << "[Sounder] " << msg << msg2 << std::endl
namespace sndr {
    void init()
    {
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 1, 2048) < 0)
            exit(-1);
    }

    class Sound 
    {
    private:
        Mix_Music *music = nullptr;
    public:
        Sound()
        {
        }

        void from_ogg(const char* path)
        {
            Mix_Init(MIX_INIT_OGG);
            music = Mix_LoadMUS(path);
            if (!music)
            {
                DBG2("Could not load file", path);
                exit(-1);
            }
            while (Mix_Init(0))
                Mix_Quit();
        }

        void play(int times)
        {
            if (music != nullptr)
                Mix_PlayMusic(music, times);
        }
        
        ~Sound()
        {
            if (music != nullptr)
                Mix_FreeMusic(music);
            else
                DBG("Freeing empty music");
        }
    };
}
