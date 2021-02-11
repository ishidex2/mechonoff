#pragma once

#include "phys.cpp"
#include "tilemap.cpp"
#include "sys_menu.cpp"
#include <iostream>
#include <string>
namespace game
{
    const int TID_DOOR = 4;
    const int TID_SPIKE = 5;               
    const int TID_JOLT = 12;         
    struct Interactible
    {
        Vec2<int> tpos;
        game::Rect<double> rect;
        enum Type 
        {
            Door,
            Spike,
            Jolt
        } type;
    };

    // Thankes, SO
    template <typename INT, typename T>
    void remove_indices(std::vector<T>& v, std::vector<INT>& rm)
    {
        std::for_each(rm.crbegin(), rm.crend(), [&v](auto index) { v.erase(begin(v) + index); });
    }


    struct World
    {
        Tilemap tilemap;
        Entities entities;
        std::vector<game::Rect<double>> solids;
        std::vector<Interactible> intebs;
        std::string name = "Error";
        Id player_id;
        bool exists = false;
        int jolts = 0;
        bool won = false;
        bool lost = false;
        SystemsMenu sys_menu;
        
        World(const char *path)
        {
            auto lv = jb::JsonFile();
            lv.from_file(path);
            player_id = 0;
            tilemap = game::Tilemap(&lv);
            for (auto i : tilemap.props)
            {
                if (i.first == "name")
                {
                    this->exists = true;
                    this->name = i.second;
                }
                if (i.first == "jolts")
                {
                    this->jolts = std::stoi(i.second);
                }
                else if (i.first == "abils")
                {
                    std::string token, str(i.second);
                    std::vector<std::string> abil_list;
                    while (token != str)
                    {
                        token = str.substr(0, str.find_first_of(" "));
                        str = str.substr(str.find_first_of(" ") + 1);
                        abil_list.push_back(token);
                    }
                    for (auto i : abil_list)
                    {
                        int id = -1;
                        if (i[0] >= '0' || i[0] <= '9')
                        {
                            id = i[0]-'0';
                        }
                        else 
                        {
                            std::cout << "Invaild sequence: id" << std::endl;
                            continue;
                        }

                        bool active = true;

                        if (i[1] == '-') active = false;
                        else if (i[1] == '+') active = true;
                        else { std::cout << "Invaild sequence: existance" << i[1] << std::endl; continue;}

                        int price = 0;
                        if (i[2] == '!') price = -1;
                        else if (i[2] >= '0' || i[2] <= '9')
                        {
                            price = i[2]-'0';
                        }
                        else 
                        {
                            std::cout << "Invaild sequence: price" << std::endl;
                            continue;
                        }

                        if (id == 0)
                        {
                            this->sys_menu.add_entry(SolidCollision, active, price);
                        }
                        else if (id == 1)
                        {
                            this->sys_menu.add_entry(Gravity, active, price);
                        }
                        else 
                        {
                            std::cout << "Invaild parameter: id" << std::endl;
                        }

                    }
                }
            }

            for (auto i = tilemap.tiles.begin(); i != tilemap.tiles.end(); ++i)
            {
                auto rect = game::Rect<double>(i->first.x*6, i->first.y*6, 6, 6);
                if (i->second == TID_DOOR)
                    this->intebs.push_back({i->first, rect, Interactible::Door});
                else if (i->second == TID_SPIKE)
                    this->intebs.push_back({i->first, rect, Interactible::Spike});
                else if (i->second == TID_JOLT)
                    this->intebs.push_back({i->first, rect, Interactible::Jolt});
                else if (i->second == 2 || i->second == 3) solids.push_back(rect);
            }
            entities.push_back(Entity{false, game::Rect<double>(tilemap.p_pos.x, tilemap.p_pos.y, 5, 6), game::Vec2<double>(0, 0), EntityType::Player});
        }

        Entity* player()
        {
            return &entities[player_id];
        }

        // Returns if collected
        bool handle_interactibles()
        {
            std::vector<unsigned int> remove;
            int i = 0;
            bool collected = false;
            for (auto inter : this->intebs)
            {
                if (player()->rect.intersects(inter.rect))
                {
                    if (inter.type == Interactible::Door)
                    {
                        this->won = true;
                        break;
                    }
                    else if (inter.type == Interactible::Spike)
                    {
                        this->lost = true;
                        break;
                    }
                    else if (inter.type == Interactible::Jolt)
                    {
                        remove.push_back(i);
                        this->jolts += 1;
                        collected = true;
                    }
                }
                i += 1;
            }

            auto begin = this->intebs.begin();
            for (auto rem : remove)
            {
                this->tilemap.tiles[this->intebs[rem].tpos] = 0;
            }
            remove_indices(this->intebs, remove);
            return collected;
        }
    }; 
}
