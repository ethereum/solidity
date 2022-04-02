contract C {
    function f() public {
        ripemd160.value();
    }
}
// ----
// TypeError 8820: (47-62='ripemd160.value'): Member "value" is only available for payable functions.
