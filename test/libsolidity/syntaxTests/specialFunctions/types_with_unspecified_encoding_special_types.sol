contract C {
    function f() public pure {
        (bool a,) = address(this).call(abi.encode(address(this).delegatecall, super));
        (a,) = address(this).delegatecall(abi.encode(log0, tx, mulmod));
        a;
    }
}
// ----
// TypeError 2056: (94-120): This type cannot be encoded.
// TypeError 2056: (122-127): This type cannot be encoded.
// TypeError 2056: (184-188): This type cannot be encoded.
// TypeError 2056: (190-192): This type cannot be encoded.
// TypeError 2056: (194-200): This type cannot be encoded.
