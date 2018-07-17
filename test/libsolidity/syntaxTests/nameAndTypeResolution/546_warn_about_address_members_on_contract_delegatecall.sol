contract C {
    function f() view public {
        this.delegatecall;
    }
}
// ----
// TypeError: (52-69): Member "delegatecall" not found or not visible after argument-dependent lookup in contract C. Use "address(this).delegatecall" to access this address member.
