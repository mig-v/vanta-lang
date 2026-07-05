#include <iostream>

#include "runtime/gc.h"

GarbageCollector::GarbageCollector()
{
	this->heapHead = nullptr;
	this->heapSize = 0;
	this->heapThreshold = DEFAULT_THRESHOLD;

	#ifdef _DEBUG
		stats.allocations = 0;
		stats.collections = 0;
		stats.frees = 0;
	#endif
}

GarbageCollector::~GarbageCollector()
{
	GCObject* head = heapHead;
	while (head)
	{
		#ifdef _DEBUG
			stats.frees++;
		#endif

		GCObject* prev = head;
		head = head->next;
		delete prev;
	}
}

void GarbageCollector::collect(std::vector<Value>& stack, std::vector<Value>& globals)
{
	#ifdef _DEBUG
		stats.collections++;
	#endif

	// mark all values in the stack and globals
	for (Value& val : stack)
		mark_value(val);

	for (Value& val : globals)
		mark_value(val);

	// sweep the gc objects and free any objects that are unreachable
	sweep();

	// grow the heapThreshold to allow more allocations before another gc cycle
	heapThreshold = std::max(DEFAULT_THRESHOLD, heapSize * 2);
}

void GarbageCollector::mark_value(Value& val)
{
	// check the kind of val, and mark any heap objects, we also need to try and mark any values that they themselves hold
	// for example, an array needs to mark any heap objects it holds
	// primitives dont need any handling since they live on the stack
	if (val.kind == ValueKind::VALUE_INSTANCE)
	{
		Instance* instance = std::get<Instance*>(val.data);
		mark_object(instance);

		for (Value& field : instance->fields)
			mark_value(field);
	}
	else if (val.kind == ValueKind::VALUE_ARR)
	{
		Array* arr = std::get<Array*>(val.data);
		mark_object(arr);

		for (Value& element : arr->arr)
			mark_value(element);
	}
}

void GarbageCollector::mark_object(GCObject* object)
{
	// nullptr / already marked, we early return
	if (!object || object->marked)
		return;

	object->marked = true;
}

void GarbageCollector::sweep()
{
	GCObject** current = &heapHead;

	while (*current)
	{
		// if current object is not marked (dead), then it's unreachable via the stack or globals, so we need to free it
		if (!(*current)->marked)
		{
			#ifdef _DEBUG
				stats.frees++;
			#endif

			GCObject* unreached = *current;
			*current = unreached->next;
			delete unreached;
			heapSize--;
		}

		// object is reachable, so it's still alive, unmark it for the next gc cycle, and go to the next object in the linked list
		else
		{
			(*current)->marked = false;
			current = &(*current)->next;
		}
	}
}

void GarbageCollector::log_stats()
{
	#ifdef _DEBUG
		std::cout 
			<< "[GC Stats]\n"
			<< "    allocations : " << stats.allocations << "\n"
			<< "    frees       : " << stats.frees << "\n"
			<< "    collections : " << stats.collections << "\n"
			<< "    live objects: " << heapSize << "\n";
	#else
		std::cout << "GC Stats not available in non-debug builds\n";
	#endif
}