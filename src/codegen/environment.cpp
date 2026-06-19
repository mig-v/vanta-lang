#include "codegen/environment.h"

Environment::Environment()
{
	// initialize the global scope
	scopeDepth = 0;
	scopes.push_back(Scope{ 0, 0 });
}
#include <iostream>

void Environment::new_scope(ScopeKind kind)
{
	// we need to track if this is simply a bare block in the middle of the source code like if blocks, while blocks, for blocks, or just a pair of {},
	// if it is, then we need to maintain the slot offset of the parent scope.
	// if it's a function level scope, then slot indices begin at 0 again
	//int slotStart = (kind == ScopeKind::Normal) ? scopes.back().nextSlot : 0;

	int slotStart = 0;

	// this new scope is normal, and the previous scope is the global scope, we start locals at 0 since all vars / fns get stored
	// in global slots for the first scope
	if (kind == ScopeKind::Normal)
	{
		if (scopes.size() == 1)
			slotStart = 0;

		// nextSlot is relative to the parent slot offset, so we need both to get the correct slot start
		else
			slotStart = scopes.back().nextSlot + scopes.back().parentSlotOffset;
	}
	else if (kind == ScopeKind::FnLevel)
	{
		slotStart = 0;
	}

	scopeDepth++;
	scopes.push_back(Scope{ slotStart, scopeDepth });
}

void Environment::end_scope()
{
	scopes.pop_back();
	scopeDepth--;
}

int Environment::add_entry(const std::string& name)
{
	Scope& scope = scopes.back();
	scope.locals.push_back(name);
	int slot = scope.parentSlotOffset + scope.nextSlot;
	scope.nextSlot++;
	return slot;
}

EnvEntry Environment::resolve_entry(const std::string& name)
{
	// iterate starting from the deepest scope depth to allow for variable shadowing in nested scopes
	for (auto it = scopes.rbegin(); it != scopes.rend(); it++)
	{
		for (size_t i = 0; i < (*it).locals.size(); i++)
		{
			if ((*it).locals[i] == name)
				return EnvEntry{ (*it).depth, it->parentSlotOffset + static_cast<int>(i) };
		}
	}

	// sentinal value for when the identifier doesn't exist in the environment
	return EnvEntry{ -1, -1 };
}

uint16_t Environment::current_scope_slot_count()
{
	return static_cast<uint16_t>(scopes.back().nextSlot + scopes.back().parentSlotOffset);
}