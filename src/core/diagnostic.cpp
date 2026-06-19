#include <iostream>

#include "core/diagnostic.h"

bool DiagnosticReporter::has_errors()
{
	return diagnostics.size() > 0;
}

void DiagnosticReporter::submit_diagnostic(const Diagnostic& diagnostic)
{
	diagnostics.emplace_back(diagnostic);
}

void DiagnosticReporter::log_diagnostics()
{
	for (const Diagnostic& diagnostic : diagnostics)
		std::cout << diagnostic.message << " line: " << diagnostic.line << " column: " << diagnostic.column << std::endl;

	diagnostics.clear();
}