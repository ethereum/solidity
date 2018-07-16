pragma experimental "v0.5.0";
contract C {
    function f() public {
        this.callcode;
    }
}
// ----
// TypeError: (77-90): Member "callcode" not found or not visible after argument-dependent lookup in contract C.
