contract C {
    /// audit: owner gate ‮
    function f() external {}
}
// ----
// ParserError 9182: (17-47): Function, variable, struct or modifier declaration expected.
