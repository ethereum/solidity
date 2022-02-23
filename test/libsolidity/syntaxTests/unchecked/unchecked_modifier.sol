contract C {
    modifier m() { unchecked { _; } }
}
// ----
// SyntaxError 2573: (44-45): The placeholder statement "_" cannot be used inside an "unchecked" block.
