contract C {
    apply public {
        function f() private {}
    }
}
// ----
// ParserError: (53-60): Cannot override modifier area's visibility.
