contract C {
    function f() public {
        ecrecover.value();
    }
}
// ----
// TypeError: (47-62): Member "value" is only available for payable functions.
