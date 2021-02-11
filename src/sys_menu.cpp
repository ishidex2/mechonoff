#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
namespace game
{
    enum System
    {
        SolidCollision,
        Gravity
    };

    struct SystemMenuEntry
    {
        System type;
        std::string name;
        bool active;
        unsigned int price;
    };

    struct SystemsMenu
    {
        int caret = 0;
        std::vector<SystemMenuEntry> entries;

        bool is_caret_active()
        {
            return this->entries[this->caret].active;
        }
      
        void add_entry(System sys, bool is_active, unsigned int jolts)
        {
            std::string name;

            switch (sys) {
                case System::SolidCollision:
                    name = "Solid Collision";
                break;
                case System::Gravity:
                    name = "Gravity";
                break;
            }

            this->entries.push_back({sys, name, is_active, jolts});
        }

        void move_caret(int dir)
        {
            this->caret += dir;
            if (this->caret < 0) this->caret = 0;
            if (this->caret >= this->entries.size()) this->caret = this->entries.size()-1;
        }
        
        bool select_entry(int *jolts)
        {
            if (this->entries[this->caret].price <= *jolts)
            {
                *jolts -= this->entries[this->caret].price;
                this->entries[this->caret].active = !this->entries[this->caret].active;
                return true;
            }
            return false;
        }
    };
}
