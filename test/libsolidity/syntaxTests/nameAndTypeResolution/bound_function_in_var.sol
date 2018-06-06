library D { struct s { uint a; } function mul(s storage self, uint x) public returns (uint) { return self.a *= x; } }
contract C {
    using D for D.s;
    D.s x;
    function f(uint a) public returns (uint) {
        var g = x.mul;
        return g({x: a});
    }
}
// ----
// Warning: (218-223): Use of the "var" keyword is deprecated.
