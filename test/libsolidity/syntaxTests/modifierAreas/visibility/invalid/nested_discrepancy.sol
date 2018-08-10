contract C {
    apply public {
        apply private {}
    }
}
// ----
// SyntaxError: (40-56): Cannot override parent modifier area's visibility.
