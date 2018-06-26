library D { struct s { uint a; } function mul(s storage self, uint x) public returns (uint) { return self.a *= x; } }
contract C {
    using D for D.s;
    D.s x;
    function f(uint a) public returns (uint) {
        function (D.s storage, uint) returns (uint) g = x.mul;
        g(x, a);
        g(a);
    }
}
// ----
// TypeError: (218-271): Type function (struct D.s storage pointer,uint256) returns (uint256) is not implicitly convertible to expected type function (struct D.s storage pointer,uint256) returns (uint256).
// TypeError: (298-302): Wrong argument count for function call: 1 arguments given but expected 2.
