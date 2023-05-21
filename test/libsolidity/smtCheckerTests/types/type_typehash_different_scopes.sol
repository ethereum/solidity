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
    function f() public pure {
        assert(A.a == B.b);
    }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
