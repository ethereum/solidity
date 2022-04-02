contract C {
    function f() public {
        sha256.value();
    }
}
// ----
// TypeError 8820: (47-59='sha256.value'): Member "value" is only available for payable functions.
