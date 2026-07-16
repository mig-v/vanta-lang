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

void GarbageCollector::collect(std::vector<Value>& stack, std::vector<Module*>& allModules)
{
	#ifdef _DEBUG
		stats.collections++;
	#endif

	// mark all values in the stack and globals
	for (Value& val : stack)
		mark_value(val);

	for (Module* module : allModules)
	{
		if (module->marked)
			continue;

		mark_object(module);

		for (Value& val : module->globals)
			mark_value(val);
	}

	// sweep the gc objects and free any objects that are unreachable
	sweep();

	// grow the heapThreshold to allow more allocations before another gc cycle
	heapThreshold = std::max(DEFAULT_THRESHOLD, heapSize * 2);
}

void GarbageCollector::mark_value(const Value& val)
{
	// check the kind of val, and mark any heap objects, we also need to try and mark any values that they themselves hold
	// for example, an array needs to mark any heap objects it holds
	// primitives dont need any handling since they live on the stack
	switch (val.kind)
	{
		case ValueKind::VALUE_INSTANCE:
		{
			Instance* instance = std::get<Instance*>(val.data);
			if (instance->marked) break;
			
			mark_object(instance);

			for (Value& field : instance->fields)
				mark_value(field);

			break;
		}
		case ValueKind::VALUE_ARR:
		{
			Array* arr = std::get<Array*>(val.data);
			if (arr->marked) break;

			mark_object(arr);

			for (Value& element : arr->arr)
				mark_value(element);

			break;
		}
		case ValueKind::VALUE_DICT:
		{
			Dict* dict = std::get<Dict*>(val.data);
			if (dict->marked) break;

			mark_object(dict);

			for (auto& pair : dict->dict)
			{
				mark_value(pair.first);
				mark_value(pair.second);
			}

			break;
		}
		case ValueKind::VALUE_FILE:
		{
			File* file = std::get<File*>(val.data);
			if (file->marked) break;

			mark_object(file);
			break;
		}
		case ValueKind::VALUE_BOUND_METHOD:
		{
			BoundMethod* method = std::get<BoundMethod*>(val.data);
			if (method->marked) break;
			
			mark_object(method);
			mark_value(method->object);

			break;
		}
		case ValueKind::VALUE_MODULE:
		{
			Module* module = std::get<Module*>(val.data);
			if (module->marked) break;

			mark_object(module);

			for (Value& global : module->globals)
				mark_value(global);

			break;
		}
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