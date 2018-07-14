contract C {
    function f() public {
        this.send;
    }
}
// ----
// TypeError: (47-56): Member "send" not found or not visible after argument-dependent lookup in contract C
