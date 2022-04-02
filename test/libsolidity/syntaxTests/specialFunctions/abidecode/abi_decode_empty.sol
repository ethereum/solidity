contract C {
    function f() public pure {
        abi.decode("abc", ());
    }
}
// ----
// Warning 6133: (52-73='abi.decode("abc", ())'): Statement has no effect.
