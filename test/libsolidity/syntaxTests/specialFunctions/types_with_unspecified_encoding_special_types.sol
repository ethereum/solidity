contract C {
    function f() public pure {
        bool a = address(this).call(abi.encode(address(this).delegatecall, super));
        bool b = address(this).delegatecall(abi.encode(log0, tx, mulmod));
        a; b;
    }
}
// ----
// TypeError: (91-117): This type cannot be encoded.
// TypeError: (119-124): This type cannot be encoded.
// TypeError: (183-187): This type cannot be encoded.
// TypeError: (189-191): This type cannot be encoded.
// TypeError: (193-199): This type cannot be encoded.
