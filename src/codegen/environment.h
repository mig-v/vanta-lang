#pragma once

#include <string>
#include <vector>

enum class ScopeKind 
{
	Normal,
	FnLevel,
};

struct Scope
{
	Scope(int parentSlotOffset, int depth) : parentSlotOffset(parentSlotOffset), depth(depth), nextSlot(0) {}

	std::vector<std::string> locals;
	int nextSlot;
	int depth;
	int parentSlotOffset;
};

struct EnvEntry
{
	EnvEntry(int scope, int slot) : scope(scope), slot(slot) {}

	int scope;
	int slot;
};

// used for tracking scope / lifetime of variables
class Environment
{
public:
	Environment();

	inline int get_scope_depth() const { return scopeDepth; }

	void new_scope(ScopeKind kind);
	void end_scope();

	int add_entry(const std::string& name);
	EnvEntry resolve_entry(const std::string& name);

	uint16_t current_scope_slot_count();

private:
	std::vector<Scope> scopes;
	int scopeDepth;

};