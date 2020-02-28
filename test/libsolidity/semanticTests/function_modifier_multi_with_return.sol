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

// ----
// f(bool): false -> 1
// f(bool): true -> 2
