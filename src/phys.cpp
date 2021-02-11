#pragma once
#include "math.cpp"
#include <vector>
#include "defs.h"

enum EntityType
{
    Player
};

struct Entity
{
    bool on_ground;
    game::Rect<double> rect;
    game::Vec2<double> vel;
    EntityType type;
};

typedef std::vector<Entity> Entities;

namespace game 
{
    void process_entities(double delta, Entities *ent, bool enable_gravity)
    {
        for (auto i = ent->begin(); i != ent->end(); ++i)
        {
            i.base()->on_ground = false;
            if (enable_gravity)
            {
                i.base()->vel.y += delta*50;
            }
            i.base()->rect.pos = i.base()->rect.pos.add(i.base()->vel.mul(delta));
            i.base()->vel.x /= 4.0;
        }
    }

    void resolve_entity_rects(std::vector<Rect<double>> *solids, Entities *entities)
    {
        for (auto m = entities->begin(); m != entities->end(); ++m)
        {
            auto vel = &m.base()->vel;
            auto prect = &m.base()->rect;
            std::sort(solids->begin(), solids->end(), [&] (const Rect<double> a, const Rect<double> b) {
                return (a.pos.add(b.size.div(2.0)).sub(prect->pos.add(prect->size.div(2.0)))).mag() < (b.pos.add(b.size.div(2.0)).sub(prect->pos.add(prect->size.div(2.0)))).mag();
            });

            for (auto i = solids->begin(); i != solids->end(); ++i) 
            {
                auto rect = *i.base();
                auto diff = prect->diff(rect);

                auto dom_axis = abs(diff.size.x) < abs(diff.size.y) ? 0 : 1;
                if (prect->intersects(rect))
                {
                    if (dom_axis == 0)
                    {
                        if (rect.pos.x > prect->pos.x)
                        {
                            prect->pos.x -= diff.size.x;
                        }
                        else
                            prect->pos.x += diff.size.x;
                    }
                    else
                    {
                        if (rect.pos.y > prect->pos.y)
                        {
                            vel->y = 0;
                            m.base()->on_ground = true;
                            prect->pos.y -= diff.size.y;
                        }
                        else
                        {
                            vel->y = 0;
                            prect->pos.y += diff.size.y;
                        }

                    }
                }
            }
        }
        
    }
}
