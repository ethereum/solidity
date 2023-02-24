struct S {
    uint x;
}

function structSuffix(uint x) pure suffix returns (S storage s) {
    assembly ("memory-safe") {
        s.slot := x
    }
}

function arraySuffix(uint x) pure suffix returns (uint[5] storage a) {
    assembly ("memory-safe") {
        a.slot := x
    }
}

function mappingSuffix(uint x) pure suffix returns (mapping(uint => uint) storage m) {
    assembly ("memory-safe") {
        m.slot := x
    }
}

contract C {
    function f() public pure {
        1 structSuffix;
        1 arraySuffix;
        1 mappingSuffix;
    }
}
// ----
// TypeError 7251: (77-88): Literal suffix functions can only return value types and reference types stored in memory.
// TypeError 7251: (202-219): Literal suffix functions can only return value types and reference types stored in memory.
// TypeError 7251: (335-366): Literal suffix functions can only return value types and reference types stored in memory.
