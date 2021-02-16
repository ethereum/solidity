contract C {
    function f() public pure {
        uint mload;
    }
    function g() public pure {
        uint mload;
        assembly {
        }
    }
}
// ----
// Warning 2072: (52-62): Unused local variable.
// Warning 2072: (109-119): Unused local variable.
