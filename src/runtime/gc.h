#pragma once

#include "codegen/value.h"

struct GCStats
{
	size_t allocations;
	size_t frees;
	size_t collections;
};

class GarbageCollector
{
public:
	GarbageCollector();
	~GarbageCollector();

	void log_stats();
	void collect(std::vector<Value>& stack, std::vector<Module*>& allModules);

	inline bool should_collect() const { return heapSize > heapThreshold; }

	template <typename T, typename... Args>
	T* alloc_object(Args&&... args)
	{
		#ifdef _DEBUG
			stats.allocations++;
		#endif

		T* obj = new T(std::forward<Args>(args)...);
		obj->next = heapHead;
		heapHead = obj;
		heapSize++;
		return obj;
	}
private:
	void mark_value(const Value& val);
	void mark_object(GCObject* object);
	void sweep();

	static constexpr size_t DEFAULT_THRESHOLD = 100;
	GCObject* heapHead;   // head object in heap object linked list
	size_t heapSize;	  // current live object
	size_t heapThreshold; // collect dead objects when this number is reached

	
#ifdef _DEBUG
	GCStats stats;
#endif
};