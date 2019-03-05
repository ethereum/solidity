contract C {
    function f() public {
        sha256.value();
    }
}
// ----
// TypeError: (47-59): Member "value" is only available for payable functions.
