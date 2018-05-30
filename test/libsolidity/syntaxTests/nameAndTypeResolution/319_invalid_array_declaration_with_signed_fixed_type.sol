contract test {
    function f() public {
        uint[fixed(3.5)] a; a;
    }
}
// ----
// TypeError: (55-65): Invalid array length, expected integer literal or constant expression.
