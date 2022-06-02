struct S {
    uint x;
}

function structSuffix(uint x) pure returns (S storage s) {
    assembly ("memory-safe") {
        s.slot := x
    }
}

function arraySuffix(uint x) pure returns (uint[5] storage a) {
    assembly ("memory-safe") {
        a.slot := x
    }
}

function mappingSuffix(uint x) pure returns (mapping(uint => uint) storage m) {
    assembly ("memory-safe") {
        m.slot := x
    }
}

contract C {
    function f() public pure {
        // TODO: Using functions with storage return type as suffixes should be disallowed
        1 structSuffix;
        1 arraySuffix;
        1 mappingSuffix;
    }
}
