contract C {
    fallback () public { }
}
// ----
// TypeError: (17-39): Fallback function must be defined as "external".
