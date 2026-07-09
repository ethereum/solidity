contract A { constructor(uint x) {} }
contract B is A(1) { constructor(uint y, uint z) {} }
contract C1 is B(2, 3) layout at 2**256 - 2**42 { constructor() {} }
contract C2 is B(2, 3) { constructor() {} }
contract F {
    function withSpecifier() public {
        new C1();
    }
    function withoutSpecifier() public {
        new C2();
    }
}
// ====
// targetContract: F
// ----
// withSpecifier() ->
// withoutSpecifier() ->
