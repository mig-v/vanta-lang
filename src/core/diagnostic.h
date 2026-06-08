#pragma once

#include <string>
#include <vector>

enum class Phase
{
	Parser,
	Semantic
};

struct Diagnostic
{
	Phase phase;
	std::string message;
	uint32_t line;
	uint32_t column;

	Diagnostic(Phase phase, const std::string& message, uint32_t line, uint32_t column) 
		: phase(phase), message(message), line(line), column(column) {}
};

class DiagnosticReporter
{
public:
	void log_diagnostics();
	void submit_diagnostic(const Diagnostic& diagnostic);
private:
	std::vector<Diagnostic> diagnostics;
};