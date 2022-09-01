library D { struct s { uint a; } function mul(s storage self, uint x) public returns (uint) { return self.a *= x; } }
contract C {
    using D for D.s;
    D.s public x;
    function f(uint a) public returns (uint) {
        x.a = 6;
        return x.mul({x: a});
    }
}
// ====
// compileToEwasm: false
// ----
// library: D
// f(uint256): 7 -> 0x2a
// x() -> 0x2a
