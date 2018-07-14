contract C {
    function f() public {
        this.call;
    }
}
// ----
// TypeError: (47-56): Member "call" not found or not visible after argument-dependent lookup in contract C
