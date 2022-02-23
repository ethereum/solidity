contract C {
    function f() public {
        while (true)
        {
        }
        continue;
    }
}
// ----
// SyntaxError 4123: (88-96): "continue" has to be in a "for" or "while" loop.
