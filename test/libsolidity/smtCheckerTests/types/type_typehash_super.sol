contract A {
    struct S {
        string x;
        bool[10][] y;
    }
}

contract C is A {
    function f() public pure {
        // keccak256("S(string x,bool[10][] y)")
        assert(type(S).typehash == 0xb4abec9b1d4b9d4724891b27b275d7d5e1692fe69fe6ff78379f613500046c11);
    }
}
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
