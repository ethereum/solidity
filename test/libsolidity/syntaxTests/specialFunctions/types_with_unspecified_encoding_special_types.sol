contract C {
    function f() public pure {
        (bool a,) = address(this).call(abi.encode(address(this).delegatecall, super));
        (a,) = address(this).delegatecall(abi.encode(block, tx, mulmod));
        a;
    }
}
// ----
// TypeError 2056: (94-120='address(this).delegatecall'): This type cannot be encoded.
// TypeError 2056: (122-127='super'): This type cannot be encoded.
// TypeError 2056: (184-189='block'): This type cannot be encoded.
// TypeError 2056: (191-193='tx'): This type cannot be encoded.
// TypeError 2056: (195-201='mulmod'): This type cannot be encoded.
