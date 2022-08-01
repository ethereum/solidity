error E(uint);
library L {
    function f(uint) internal {}
}
contract C {
    using L for E;
    function f() public pure {
        E.f();
    }
}
// ----
// TypeError 5172: (91-92): Name has to refer to a user-defined value type, struct, enum or contract.
