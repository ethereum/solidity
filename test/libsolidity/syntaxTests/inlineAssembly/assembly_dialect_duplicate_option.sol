function f() pure {
    assembly "evmasm" ("memory-safe", "memory-safe") {}
}
// ----
// SyntaxError 7026: (24-75): Inline assembly marked memory-safe multiple times.
