contract test {
    function f() public {
        bytes32 c = 3.2; c;
    }
}
// ----
// TypeError 9574: (50-65): Type rational_const 16 / 5 is not implicitly convertible to expected type bytes32.
