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
        x.a = 6;
        return x.mul({
            x: a
        });
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
        x.a = 6;
        return x.mul({
            x: a
        });
    }
}

// ----
// f(uint256): 7) -> 6 * 7
// f(uint256):"7" -> "42"
// x() -> 6 * 7
// x():"" -> "42"
