contract C {
    function f() public pure {
        unchecked {
            unchecked {
                uint x = 2 + 3;
            }
        }
    }
}
// ----
// SyntaxError 1941: (76-133): "unchecked" blocks cannot be nested.
