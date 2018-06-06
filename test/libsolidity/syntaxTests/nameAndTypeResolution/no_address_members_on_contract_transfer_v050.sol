pragma experimental "v0.5.0";
contract C {
    function f() public {
        this.transfer;
    }
}
// ----
// TypeError: (77-90): Member "transfer" not found or not visible after argument-dependent lookup in contract C
