pragma experimental "v0.5.0";
contract C {
    function f() public {
        this.balance;
    }
}
// ----
// TypeError: (77-89): Member "balance" not found or not visible after argument-dependent lookup in contract C
