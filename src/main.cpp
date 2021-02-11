#include <cctype>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "sounder.cpp"
#include "json_builder.cpp"
#include "math.cpp"
#include "phys.cpp"
#include "tilemap.cpp"
#include "game_state.cpp"
#include "sys_menu.cpp"
#include "world.cpp"

#define COL_FORE olc::Pixel(0x43, 0x52, 0x3d)
#define COL_BACK olc::Pixel(0xc7, 0xf0, 0xd8)

namespace game {
    struct Camera
    {
        Vec2<double> pos;
        double zoom = 0.0;
    };
}

struct Particle
{
    game::Vec2<double> pos;
    game::Vec2<double> vel;

    double life;
};

class Example : public olc::PixelGameEngine
{
    double seconds = 0;
    double deaths = 0;
    double animation_state = 0;
    int animation_incr = 0;
    std::vector<Particle> particles;
    game::World *world = nullptr;
    olc::Sprite *tiles;
    olc::Sprite *end_screen;
    sndr::Sound win_sound;
    sndr::Sound rocket;
    sndr::Sound collect_sound;
    sndr::Sound select_sound;
    sndr::Sound submit_sound;
    GameState state = TitleScreen;
    game::Camera cam;
    const char *lvls[10] {
        "assets/Level1.json",
        "assets/Level2.json",
        "assets/Level3.json",
        "assets/Level4.json",
        "assets/Level5.json",
        "assets/Level6.json",
        "assets/Level7.json",
        "assets/Level8.json",
        "assets/Level9.json",
        "assets/Level10.json",
    };
    uint32_t tint_layer;
    unsigned int lvl_handle = 0; 
    double total = 0;

public:
	Example()
	{
		sAppName = "Example";
	}

public:
    void load_stage()
    {
       if (this->world != nullptr)
            delete this->world;

       this->world = new game::World(lvls[lvl_handle]); 
    }

    void next_level()
    {
        lvl_handle += 1;
        if (lvl_handle > 9) switch_state(Ending);
        else load_stage();
    }

    void switch_state(GameState to)
    {
        this->state = lvl_handle > 9 ? Ending : to;
        this->animation_state = 0;
        this->animation_incr = 0;
    }

    void suddenly_at(int id)
    {
        this->lvl_handle = id;
        load_stage();
        switch_state(Gameplay);
    }

	bool OnUserCreate() override
	{
        sndr::init();
        this->tint_layer = CreateLayer();
        rocket.from_ogg("assets/launch.ogg");
        collect_sound.from_ogg("assets/collect.ogg");
        win_sound.from_ogg("assets/sound.ogg");
        select_sound.from_ogg("assets/select.ogg");
        submit_sound.from_ogg("assets/submit.ogg");
        tiles = new olc::Sprite("assets/tiles.png");
        end_screen = new olc::Sprite("assets/end.png");
        // Called once at the start, so create things here
		return true;
    }

    void FillRectEx(double x, double y, double w, double h, olc::Pixel p = COL_FORE)
    {
        FillRect(round((x-this->cam.pos.x)*cam.zoom), round((y-this->cam.pos.y)*cam.zoom), ceil(w*cam.zoom), ceil(h*cam.zoom), p);
    }

    void DrawPartialSpriteEx(double x, double y, olc::Sprite *spr, int tx, int ty, int tw, int th, bool flip = false)
    {
        DrawPartialSprite(round((x-this->cam.pos.x+sin(total)*(lvl_handle/5))*cam.zoom), round((y-this->cam.pos.y)*cam.zoom), spr, tx, ty, tw*std::min(cam.zoom, 1.), th*std::min(cam.zoom, 1.), cam.zoom, flip ? olc::Sprite::HORIZ : olc::Sprite::NONE);
    }

    int str_center(const char *s)
    {
        return int(ScreenWidth()/2)-int(strlen(s)*4/2);
    }

    void DrawStringCustomWiggle(int x, int y, std::string txt)
    {
        int base_offs = x;
        for (auto i : txt)
        {
            DrawStringCustom(base_offs, y+sin(total+base_offs/2.0)*2.0, {i, '\0'});
            base_offs += i == 'w' ? 6 : 4;
        }
    }

    void DrawStringCustomEx(double x, double y, std::string txt)
    {
        DrawStringCustomEx(x, y, txt.c_str());
    }

    void DrawStringCustomEx(double x, double y, const char *txt)
    {
        if (cam.zoom < 1) return;
        DrawStringCustom(std::round((x-cam.pos.x)/cam.zoom), std::round((y-cam.pos.y)/cam.zoom), txt);
    }

    void DrawStringCustom(double x, double y, std::string txt)
    {
        DrawStringCustom(x, y, txt.c_str());
    }

    void DrawStringCustom(double x, double y, const char *txt, olc::Pixel color = COL_FORE)
    {
        std::string s = txt;
        int _x = x;
        int _y = y;
        for (auto chr : s)
        {
            auto lw = tolower(chr);
            int sz = lw > 117 ? 6 : 4;
            if (lw >= 97 && lw <= 122)
            {
                int xoff = lw <= 117 ? (lw-97)*sz : (lw-118)*sz;
                int yoff = 12 + (lw > 117 ? 6 : 0);
                DrawPartialSprite(_x, _y, tiles, xoff, yoff, sz, 6);
            }

            _x += lw == 'w' ? 6 : 4;
        }
    }

    void DrawTransition(double state)
    {
        for (int i = 0; i < 20; i += 1)
        {
            for (int j = 0; j < 10; j += 1)
            {
                DrawPartialSprite(i*6, j*6, tiles, 48, 18+std::min(42-18, int(state*6)), 6, 6);
            }
        }
    }
	bool OnUserUpdate(float fElapsedTime) override
	{
        Clear(COL_BACK);
        if (!IsFocused())
        {
            DrawStringCustom(0, 0, "Please Focus on the");
            DrawStringCustom(0, 6, "game window");
            DrawStringCustom(0, 18, "Click on the");
            DrawStringCustom(0, 24, "game frame");
            return true;
        }
        SetPixelMode(olc::Pixel::Mode::ALPHA);
        total += fElapsedTime;
        using namespace game;
        switch (this->state)
        {
            case GameState::Ending:
            {
                animation_state += fElapsedTime;
                DrawPartialSprite(0, 0, end_screen, 0, 0, 84, 48);
                if (animation_state > 2.0 && animation_incr == 0)
                {
                    this->rocket.play(1);
                    animation_incr += 1;
                }
                if (animation_state > 1.9)
                {
                    DrawPartialSprite(0, 0, end_screen, 0, 48, 84, 48);
                }
                if (animation_state > 4.0)
                {
                    if (animation_incr == 1)
                    {
                        this->select_sound.play(2);
                        animation_incr += 1; 
                    }
                    DrawPartialSprite(0, 0, end_screen, 0, 48*2, 84, 48);
                    DrawStringCustom(0, 126-48*2, "time");
                    DrawString(30, 126-48*2, std::to_string((int) floor(seconds/60))+"m "+std::to_string((int) (floor(seconds))%60)+"s", COL_FORE);
                    DrawStringCustom(0, 126+9-48*2, "deaths");
                    DrawString(30, 126+9-48*2, std::to_string((int) deaths), COL_FORE);
                }
            } break;
            case GameState::TitleScreen:
            {
                animation_state += fElapsedTime;
                // Render Mr. Mechanoff
                int ox = 10;
                
                int oy = std::min(0., ((animation_state*100)-100) * (animation_incr == 3 ? -1 : 1));
                if (GetKey(olc::Z).bReleased && animation_incr == 2)
                {
                    animation_state = 0;
                    submit_sound.play(1);
                    animation_incr = 3;
                }
                
                if (oy == 0 && animation_incr == 0)
                {
                    submit_sound.play(1);
                    animation_incr = 1;
                }
                if (animation_incr > 0)
                {
                    DrawStringCustom(45, 18, "z to play"); 
                }
                if (animation_incr == 1 && animation_state > 1.5)
                {
                    if (animation_state > 1.5)
                    {
                        DrawPartialSprite(ox+11, oy, tiles, 54, 42-6, 12, 6);
                    }
                    if (animation_state > 1.9)
                    {
                        select_sound.play(1);
                        animation_incr = 2;
                    }
                }
                else
                {
                    DrawPartialSprite(ox+11, oy, tiles, 54, 42, 12, 6);
                }
                DrawPartialSprite(ox+31, oy, tiles, 54, 42, 12, 6);
                DrawPartialSprite(ox+23, oy+10, tiles, 66, 42, 6, 6);
                DrawPartialSprite(ox+23, oy+20, tiles, 72, 42, 6, 6);
                DrawPartialSprite(ox, oy+30, tiles, 0, 30, 47, 47-29);
                
                if (animation_incr == 3 && animation_state > 1.0)
                {
                    DrawTransition((animation_state-1.0)*10.0);   
                }
                if (animation_incr == 3 && animation_state > 3.0)
                {
                    this->load_stage();
                    this->switch_state(LevelName);
                }
            } break;
            case GameState::LevelName:
            {
                this->cam.zoom = 0.1;
                this->animation_state += fElapsedTime;
                DrawStringCustomWiggle(str_center(this->world->name.c_str()), 20, this->world->name);
                if (!world->exists)
                    DrawStringCustomWiggle(str_center("bad world file"), 10, "bad world file");
                if (animation_state > 2.0 && world->exists)
                {
                    this->switch_state(Gameplay);
                }
            } break;
            case GameState::Gameplay:
            {
                seconds += fElapsedTime;
                if (world->won)
                {
                    animation_state += fElapsedTime;

                    if (animation_state >= 0.5)
                    {
                        DrawTransition((animation_state-0.5)*10);
                    }
                    if (animation_state >= 1.0)
                    {
                        this->win_sound.play(1);
                        this->next_level();
                        this->switch_state(LevelName);
                    }
                }
                cam.zoom += fElapsedTime;
                if (cam.zoom > 1.0) cam.zoom = 1.0;
                
                auto ix = 0;
                if (GetKey(olc::DOWN).bHeld) cam.pos.y += fElapsedTime*20.0;
                if (GetKey(olc::UP).bHeld) cam.pos.y -= fElapsedTime*20.0;
                if (!world->won && !world->lost)
                {
                    if (GetKey(olc::X).bPressed)
                        state = Systems;
                    if (GetKey(olc::C).bPressed)
                    {
                        load_stage();
                    }
                    if (GetKey(olc::LEFT).bHeld)
                        world->player()->vel.x -= 20;
                    if (GetKey(olc::RIGHT).bHeld)
                        world->player()->vel.x += 20;
                    if (GetKey(olc::Key::Z).bPressed && world->player()->on_ground)
                    {
                       world->player()->vel.y = -65;
                       select_sound.play(1);
                    }
                }
                if ((world->player()->rect.pos.y > 1000 || world->lost) && animation_incr != 30)
                {
                    deaths += 1; 
                    this->rocket.play(1);
                    for (int i = 0; i < 10; i += 1)
                    {
                        auto vel = Vec2<double>(rand()%100-50, rand()%100-50);
                        this->particles.push_back({.vel = vel, .pos = world->player()->rect.pos, .life = 0});
                    }
                    animation_incr = 30;
                }
                if (world->player()->rect.pos.add({-double(ScreenWidth()/2), -double(ScreenHeight()/2-8)}).sub(cam.pos).mag() > 5)
                    cam.pos = cam.pos.add(world->player()->rect.pos.add({-double(ScreenWidth()/2), -double(ScreenHeight()/2-8)}).sub(cam.pos).mul(fElapsedTime*3.0));
                
                bool enable_gravity = false;
                bool enable_solid_collisions = false;
                
                for (auto sys : world->sys_menu.entries)
                {
                    if (sys.active)
                    {
                        switch (sys.type)
                        {
                            case SolidCollision:
                                enable_solid_collisions = true;
                                break;
                            case Gravity:
                                enable_gravity = true;
                                break;
                        }
                    }
                }

                if (!enable_gravity)
                {
                    world->player()->vel.y /= 1.05;
                    if (world->player()->vel.y > 0)
                        world->player()->vel.y = 0;
                }


                if (!world->won && !world->lost)
                    process_entities(fElapsedTime, &world->entities, enable_gravity);
                if (enable_solid_collisions)
                    resolve_entity_rects(&world->solids, &world->entities);

                

                if (world->handle_interactibles())
                {
                    animation_incr = 10;
                    animation_state = 0;
                    this->collect_sound.play(1);
                }
                
                for (auto t : world->tilemap.texts)
                {
                    DrawStringCustomEx(t.first.x, t.first.y, t.second);
                }
                    
                for (auto &t : world->tilemap.tiles)
                {
                    auto pos = t.first.mul(6);
                    if (t.second == 0) continue;
                    auto abs_offs = (t.second-1)*6;
                    DrawPartialSpriteEx(pos.x, pos.y, tiles, abs_offs % 84, 6*(abs_offs / 84), 6, 6);
                }

                for (auto m = world->entities.begin(); m != world->entities.end(); ++m)
                {
                    if (world->player()->rect.pos.y > 1000 || world->lost) break;
                    auto prect = m.base()->rect;
                    switch (m.base()->type)
                    {
                        case EntityType::Player:
                            // VERY BAD
                            srand(int(total*10.0));
                            DrawPartialSpriteEx(prect.pos.x, prect.pos.y+sin(total*5.0*!world->won), tiles, 0, 0, 6, 6);
                            if (GetKey(olc::LEFT).bHeld || GetKey(olc::RIGHT).bHeld)
                            DrawPartialSpriteEx(prect.pos.x, prect.pos.y, tiles, 6, 6, 6, 6, rand()%2);
                            else DrawPartialSpriteEx(prect.pos.x, prect.pos.y, tiles, 0, 6, 6, 6);
                        break;
                    }
                }
                if (animation_incr == 10 && animation_state < 1)
                {
                    animation_state += fElapsedTime;
                    auto tp = world->player()->rect.pos;
                    tp.x -= 6;
                    tp.y -= 12;
                    DrawPartialSpriteEx(tp.x, tp.y-animation_state*10.0, tiles, 72, 0, 12, 12);
                }
                if (animation_incr == 10 && animation_state > 1) 
                {
                    animation_state = 0;
                    animation_incr = 0;
                }
                if (animation_incr == 30)
                {
                    animation_state += fElapsedTime;
                    for (auto &p : particles)
                    {
                        if (p.life > 1.2) break;
                        p.pos.x += p.vel.x * fElapsedTime;
                        p.pos.y += p.vel.y * fElapsedTime;
                        p.vel.x /= 1.02;
                        p.vel.y /= 1.02;

                        p.life += fElapsedTime*(rand()%4+1);
                        DrawPartialSpriteEx(p.pos.x, p.pos.y, tiles, p.life < 0.3 ? 36 : (36+6), 6, 6, 6, p.vel.x < 0);
                    }
                    if (animation_state > 2.0)
                    {
                        particles.clear();
                        animation_incr = 0;
                        animation_state = 0;
                        load_stage();
                    }
                }
            } break;
            case GameState::Systems:
            {
                seconds += fElapsedTime;
                if (GetKey(olc::UP).bPressed)
                {
                    this->world->sys_menu.move_caret(-1);
                    select_sound.play(1);
                }
                if (GetKey(olc::DOWN).bPressed)
                {
                    this->world->sys_menu.move_caret(1);
                    select_sound.play(1);
                }
                if (GetKey(olc::Z).bPressed)
                {
                    if (this->world->sys_menu.select_entry(&world->jolts))
                        submit_sound.play(1);
                    else
                        select_sound.play(1);
                }
                if (GetKey(olc::X).bPressed)
                {
                    state = Gameplay;
                    break;
                }
                int coffs = -(this->world->sys_menu.caret * 8-ScreenHeight()/2)-8;
                int idx = 0;
                DrawStringCustom(10, coffs-4, "Toggle Mechanics");
                DrawPartialSprite(3, this->world->sys_menu.caret*8+sin(total*5.0)+4+coffs, tiles, 42 + (this->world->sys_menu.is_caret_active() ? 0 : 6), 0, 6, 6);
                idx = 0;
                for (auto i : this->world->sys_menu.entries)
                {
                    if (idx != this->world->sys_menu.caret)
                    {
                        if (i.active)
                            DrawPartialSprite(3, idx*8+sin(total*5.0)+4+coffs, tiles, 54, 0, 6, 6);
                        else
                            DrawPartialSprite(3, idx*8+sin(total*5.0)+4+coffs, tiles, 60, 0, 6, 6);
                    }
                    if (i.price != -1)
                        DrawString(12, idx*8+4+coffs, std::to_string(i.price), COL_FORE);
                    DrawStringCustom(20, idx*8+4+coffs, i.name);
                    idx += 1;
                }
                FillRect(0, 0, ScreenWidth(), 8, COL_FORE);
                DrawString(0, 0, std::to_string(world->jolts)+" jolts", COL_BACK);
            }
            break;
        }

        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000/100));
		return true;
	}

    ~Example()
    {
        delete tiles;
    }
};


int main()
{
	Example demo;
	if (demo.Construct(84, 48, 8, 8))
		demo.Start();

	return 0;
}

