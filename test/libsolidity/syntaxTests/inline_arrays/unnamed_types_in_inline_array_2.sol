contract C {
    function f() public {
        [type(C)];
    }
}
// ----
// TypeError 9563: (48-55): Invalid mobile type.
