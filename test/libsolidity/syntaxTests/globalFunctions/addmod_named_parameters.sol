contract C {
    function f() public pure returns (uint) {
        return addmod({k: 5, y: 3, x: 7});
    }
}

// ----
// TypeError 4974: (74-100): Named argument "k" does not match function declaration.
// TypeError 4974: (74-100): Named argument "y" does not match function declaration.
// TypeError 4974: (74-100): Named argument "x" does not match function declaration.
