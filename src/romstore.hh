#pragma once

#include <string>
#include <list>

namespace GBEmu
{

struct RomData
{
    RomData(int id, std::string name, size_t size, const void *data)
    :id_(id), name_(name), size_(size), data_(data)
    { }

    const int id_;
    const std::string name_;
    const size_t size_;
    const void* const data_;
};

class RomStore
{
public:
    RomStore();
    virtual ~RomStore();

    void AddFromFile(const std::string& filename);

    const std::list<const RomData*>& GetRoms() const { return roms_; };

    const RomData* GetRom(int id) const
    {
        for(const RomData* rom : roms_)
            if (rom->id_ == id)
                return rom;
        return nullptr;
    }

private:

    std::list<const RomData*> roms_;
    int nextId_;

};

}