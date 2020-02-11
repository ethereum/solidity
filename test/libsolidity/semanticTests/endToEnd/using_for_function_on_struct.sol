library D {
    struct s {
        uint a;
    }

    function mul(s storage self, uint x) public returns(uint) {
        return self.a *= x;
    }
}
contract C {
    using D
    for D.s;
    D.s public x;

    function f(uint a) public returns(uint) {
        x.a = 3;
        return x.mul(a);
    }
}

// ----

library D {
    struct s {
        uint a;
    }

    function mul(s storage self, uint x) public returns(uint) {
        return self.a *= x;
    }
}
contract C {
    using D
    for D.s;
    D.s public x;

    function f(uint a) public returns(uint) {
        x.a = 3;
        return x.mul(a);
    }
}

// ----
// f(uint256): 7) -> 3 * 7
// f(uint256):"7" -> "21"
// x() -> 3 * 7
// x():"" -> "21"
