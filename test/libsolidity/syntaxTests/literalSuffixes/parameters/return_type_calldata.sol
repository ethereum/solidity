struct S {
    uint x;
}

function structSuffix(uint x) pure suffix returns (S calldata s) {
    assembly {
        s := x
    }
}

function arraySuffix(uint x) pure suffix returns (uint[5] calldata a) {
    assembly {
        a := x
    }
}

contract C {
    function f() public pure {
        1 structSuffix;
        1 arraySuffix;
    }
}
// ----
// TypeError 7251: (77-89): Literal suffix functions can only return value types and reference types stored in memory.
// TypeError 7251: (182-200): Literal suffix functions can only return value types and reference types stored in memory.
