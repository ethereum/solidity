function zero() pure returns (uint) { return 1; }

contract C {
    function f() public pure {
        1 zero;
        1.1 zero;
        "a" zero;
    }
}
// ----
// TypeError 4778: (103-109): Functions that take no arguments cannot be used as literal suffixes.
// TypeError 4778: (119-127): Functions that take no arguments cannot be used as literal suffixes.
// TypeError 4778: (137-145): Functions that take no arguments cannot be used as literal suffixes.
