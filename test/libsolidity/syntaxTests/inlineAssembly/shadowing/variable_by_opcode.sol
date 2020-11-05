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
// Warning 8261: (109-119): Variable is shadowed in inline assembly by an instruction of the same name
// Warning 2072: (52-62): Unused local variable.
// Warning 2072: (109-119): Unused local variable.
