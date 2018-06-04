pragma experimental "v0.5.0";
contract C {
    function f() public {
        this.send;
    }
}
// ----
// TypeError: (77-86): Member "send" not found or not visible after argument-dependent lookup in contract C
