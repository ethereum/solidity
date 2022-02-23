contract C {
    modifier f() override override {}
}
// ----
// ParserError 9102: (39-47): Override already specified.
