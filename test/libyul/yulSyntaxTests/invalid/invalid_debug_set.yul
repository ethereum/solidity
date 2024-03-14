object "object" {
    code {
        /// @debug.set {"HELLO": "WORLD
    }
}
// ----
// SyntaxError 5721: (37-68): @debug.set: Could not parse debug data: parse error at line 1, column 17: syntax error while parsing value - invalid string: missing closing quote; last read: '"WORLD'
