// Note that return sets the return variable and jumps to the end of the current function or
// modifier code block.
contract C {
    modifier repeat(bool twice) {
        if (twice) _;
        _;
    }

    function f(bool twice) public repeat(twice) returns (uint256 r) {
        r += 1;
        return r;
    }
}
// via yul disabled because the return variables are
// fresh variables each time, while in the old code generator,
// they share a stack slot when the function is
// invoked multiple times via `_`.

// ====
// compileViaYul: false
// ----
// f(bool): false -> 1
// f(bool): true -> 2
