contract C {
    function f() public pure {
        bool a = address(this).call(address(this).delegatecall, super);
        bool b = address(this).delegatecall(log0, tx, mulmod);
        a; b;
    }
}
// ----
// TypeError: This type cannot be encoded.
// TypeError: This type cannot be encoded.
// TypeError: This type cannot be encoded.
// TypeError: This type cannot be encoded.
// TypeError: This type cannot be encoded.
