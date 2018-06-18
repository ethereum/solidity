contract C {
    apply public {
        apply private {}
    }
}
// ----
// ParserError: (46-53): Cannot override parent modifier area's visibility of "public".
