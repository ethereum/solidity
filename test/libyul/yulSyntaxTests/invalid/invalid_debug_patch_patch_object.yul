object "object" {
    code {
        /// @debug.patch { "op": "unknown_operation", "path": "/variable_a", "value": ["test"] }
    }
}
// ----
// SyntaxError 9426: (37-125): @debug.patch: Could not patch debug data: parse error: operation value 'unknown_operation' is invalid
