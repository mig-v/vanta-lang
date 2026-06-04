#pragma once

inline constexpr size_t DEFAULT_ARENA_BLOCK_SIZE = 4096;

class ArenaBlock
{
public:
	ArenaBlock(size_t blockSize)
	{
		this->blockSize = blockSize;
		this->offset = 0;
		this->buffer = new char[blockSize];
	}

	~ArenaBlock()
	{
		delete[] buffer;
	}

	template<typename T, typename... Args>
	T* tryAlloc(Args&&... args)
	{
		size_t alignedOffset = (offset + alignof(T) - 1) & ~(alignof(T) - 1);

		if (alignedOffset + sizeof(T) > blockSize)
			return nullptr;

		T* node = new (buffer + alignedOffset) T(std::forward<Args>(args)...);
		offset = alignedOffset + sizeof(T);
		return node;
	}
private:
	char* buffer;
	size_t blockSize;
	size_t offset;
};