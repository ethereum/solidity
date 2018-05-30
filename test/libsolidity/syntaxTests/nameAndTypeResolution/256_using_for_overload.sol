library D {
    struct s { uint a; }
    function mul(s storage self, uint x) public returns (uint) { return self.a *= x; }
    function mul(s storage, bytes32) public returns (bytes32) { }
}
contract C {
    using D for D.s;
    D.s x;
    function f(uint a) public returns (uint) {
        return x.mul(a);
    }
}
// ----
// Warning: (128-189): Function state mutability can be restricted to pure
