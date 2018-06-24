pragma experimental "v0.5.0";
contract C {
    function f() public {
        this.call;
    }
}
// ----
// TypeError: (77-86): Member "call" not found or not visible after argument-dependent lookup in contract C
