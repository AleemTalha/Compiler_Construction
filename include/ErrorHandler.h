// ============================================================
// ErrorHandler.h -- Centralized Error Reporting
// ============================================================
// Phase 6: Collects and prints errors from all compiler phases
// (lexical, syntax, semantic) with line:column context.
// ============================================================

#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <iostream>
#include <string>

using namespace std;

const int MAX_ERRORS = 100;

// A single compiler error with location and description
struct CompilerError {
    int line;
    int col;
    string phase;       // "Lexical", "Syntax", or "Semantic"
    string message;
};

class ErrorHandler {
private:
    CompilerError errors[MAX_ERRORS];   // Fixed array of errors
    int errorCount;

public:
    ErrorHandler() : errorCount(0) {}

    // Add a new error to the collection
    void addError(int line, int col, const string& phase, const string& message) {
        if (errorCount < MAX_ERRORS) {
            errors[errorCount].line = line;
            errors[errorCount].col = col;
            errors[errorCount].phase = phase;
            errors[errorCount].message = message;
            errorCount++;
        }
    }

    // Print all collected errors to stderr
    void printErrors() const {
        for (int i = 0; i < errorCount; i++) {
            cerr << "[" << errors[i].line << ":" << errors[i].col << "] "
                 << errors[i].phase << " Error: " << errors[i].message << endl;
        }
    }

    bool hasErrors() const {
        return errorCount > 0;
    }

    void clear() {
        errorCount = 0;
    }
};

#endif
