#include "romstore.hh"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstdint>

#include <iostream>
#include <fstream>
#include <exception>

namespace GBEmu
{

RomStore::RomStore()
    :roms_({}),
    nextId_(1)
{
}

RomStore::~RomStore()
{
}

void RomStore::AddFromFile(const std::string& filename)
{
    // Open file.
	std::ifstream stream(filename, std::ios_base::binary);

	if (!stream.is_open() || stream.bad())
	{
		printf("unable to open rom file: %s\n", filename.c_str());
		throw new std::runtime_error("unable to open rom file");
	}
	
	// Get file size.
	auto startPosition = stream.tellg();
	stream.seekg(0, std::ios_base::end);
	auto endPosition = stream.tellg();
	stream.seekg(0, std::ios_base::beg);

	const size_t fileSize = size_t(endPosition - startPosition);

	assert(fileSize > 0);
	assert(!(fileSize % (32 * 1024)));

	// Read file content.
    char *data = new char[fileSize];
    assert(data);
	stream.read(data, fileSize);
    
    // Add to store.    
    int id = nextId_++;
    std::string name = filename;

    RomData *rom = new RomData(id, name, fileSize, data);
    roms_.push_back(rom);

    printf("Added ROM from file '%s': %lu bytes\n", filename.c_str(), fileSize);
}

}