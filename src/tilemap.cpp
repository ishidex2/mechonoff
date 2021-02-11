#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include "json_builder.cpp"
#include "defs.h"
#include "math.cpp"

namespace game 
{
    using namespace jb;
    class Tilemap
    {
    public:
        std::unordered_map<Vec2<int>, Id, std::hash<Vec2<int>>> tiles;
        std::unordered_map<std::string, std::string> props;
        std::unordered_map<Vec2<double>, std::string> texts;
        Vec2<double> p_pos;

        Tilemap()
        {}
            
        Tilemap(jb::JsonFile *f)
        {
            auto props = JsonValue(f->get_handle())
                .at("properties");
            
            for (int i = 0; i < props.size(); i += 1)
            {
                auto prop = props.at(i);
                if (prop.at("type").str() == "string")
                    this->props[prop.at("name").str()] = prop.at("value").str();
                else
                    std::cout << "Omitting prop" << std::endl;
            }
            
            auto layers = JsonValue(f->get_handle())
                .at("layers");
            for (int k = 0; k < layers.size(); k++)
            {
                if (layers.at(k).at("type").str() == "objectgroup")
                {
                    auto layer = layers.at(k).at("objects");
                    for (int i = 0; i < layer.size(); i += 1)
                    {

                        auto obj = layer.at(i);
                        this->texts[Vec2<double>(obj.at("x").num(), obj.at("y").num())] = obj.at("name").str();
                    }
                }
                else
                {
                    auto layer = layers.at(k).at("chunks");
                    for (int i = 0; i < layer.size(); i += 1)
                    {
                        auto chunk = layer.at(i);
                        auto data = chunk.at("data");
                        auto width = chunk.at("width").num();
                        auto x = chunk.at("x").num();
                        auto y = chunk.at("y").num();

                        for (int j = 0; j < data.size(); j += 1)
                        {
                            auto id = data.at(j).num();
                            if (id == 0) continue;
                            if (id == 1)
                                p_pos = Vec2<double>(x+(j%width), y+(j/width)).mul(6);
                            else
                                tiles[Vec2<int>(x+(j%width), y+(j/width))] = id;
                        }
                    }
                }
            }
            
        }
    };
}
