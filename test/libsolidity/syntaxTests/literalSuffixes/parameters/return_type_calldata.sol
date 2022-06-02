struct S {
    uint x;
}

function structSuffix(uint x) pure returns (S calldata s) {
    assembly ("memory-safe") {
        s := x
    }
}

function arraySuffix(uint x) pure returns (uint[5] calldata a) {
    assembly ("memory-safe") {
        a := x
    }
}

contract C {
    function f() public pure {
        // TODO: Using functions with calldata return type as suffixes should be disallowed
        1 structSuffix;
        1 arraySuffix;
    }
}
