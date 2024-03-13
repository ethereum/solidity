object "object" {
    code {
        /// @debug.patch {"HELLO": invalid
    }
}
// ----
// SyntaxError 5721: (37-71): @debug.patch: Could not parse debug data: parse error at line 1, column 11: syntax error while parsing value - unexpected end of input; expected '[', '{', or a literal
