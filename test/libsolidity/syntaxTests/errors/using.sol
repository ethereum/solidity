error E(uint);
library L {
    function f(uint) internal {}
}
contract C {
    using L for *;
    function f() public pure {
        E.f();
    }
}
// ----
// TypeError 9582: (133-136): Member "f" not found or not visible after argument-dependent lookup in function (uint256) pure.
