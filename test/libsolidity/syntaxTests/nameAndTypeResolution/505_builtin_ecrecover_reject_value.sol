contract C {
    function f() public {
        ecrecover.value();
    }
}
// ----
// TypeError 8820: (47-62='ecrecover.value'): Member "value" is only available for payable functions.
