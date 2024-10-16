library A {
    struct S1 {
        uint256 x;
    }

    bytes32 internal constant a = type(S1).typehash;
}

library B {
    struct S1 {
        uint256 x;
    }

    bytes32 internal constant b = type(S1).typehash;
}

contract C {
    function f() public pure returns(bool) {
        return A.a == B.b;
    }
}
// ----
// f() -> true
