contract C {
    function f() public {
        if (true)
            break;
    }
}
// ----
// SyntaxError 6102: (69-74): "break" has to be in a "for" or "while" loop.
