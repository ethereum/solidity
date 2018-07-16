pragma experimental "v0.5.0";
contract C {
    function f() public {
        this.delegatecall;
    }
}
// ----
// TypeError: (77-94): Member "delegatecall" not found or not visible after argument-dependent lookup in contract C.
