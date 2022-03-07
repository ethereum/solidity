function f() pure {
    assembly ("memory-safe", "memory-safe") {}
}
// ----
// SyntaxError 7026: (24-66): Inline assembly marked memory-safe multiple times.
