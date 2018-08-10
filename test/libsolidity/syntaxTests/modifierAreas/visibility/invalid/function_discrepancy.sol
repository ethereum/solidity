contract C {
    apply public {
        function f() private {}
    }
}

// ----
// SyntaxError: (40-63): Cannot override modifier area's visibility.
