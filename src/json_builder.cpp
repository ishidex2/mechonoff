#pragma once
#define DBG(msg) std::cout << "[JsonBuilder] " << msg << std::endl
#define DBG2(msg, msg2) std::cout << "[JsonBuilder] " << msg << msg2 << std::endl
#include <iostream>

extern "C" {
    #include "json.c"
}

namespace jb 
{
    class JsonFile
    {
        private:
            JSON_Value *root;
        public:

        JsonFile()
        {
            
        }

        JSON_Value* get_handle()
        {
            return this->root;
        }

        void from_file(const char* path)
        {
            root = json_parse_file(path);
        }

        ~JsonFile()
        {
            json_value_free(root);
        }
    };

    class JsonValue 
    {
        private:
            JSON_Value *handle;
        public:
        JsonValue(JSON_Value *handle)
        {
            this->handle = handle;
        }

        JsonValue build(JsonFile *f)
        {
            this->handle = f->get_handle();
            return *this;
        }

        std::string str()
        {
            return std::string(json_value_get_string(handle));
        }

        bool boolean()
        {
            return json_value_get_boolean(handle);
        }

        int num()
        {
            return json_value_get_number(handle);
        }

        int size()
        {
            return json_array_get_count(json_value_get_array(handle));
        }

        JsonValue at(unsigned int idx)
        {
            JSON_Array *ob = json_value_get_array(handle);
            return JsonValue(json_array_get_value(ob, idx));
        }

        JsonValue at(const char* v) const
        {
            JSON_Object *ob = json_value_get_object(handle);
            return JsonValue(json_object_get_value(ob, v));
        }
    };
}
