contract Test {
    function assignment() public {
        uint8 uint8Min = type(int).min;
        uint uintMin = type(int).min;

        if (type(int).min == 2**256 - 1) {
            uintMin;
        }

    }
}
// ----
// TypeError 9574: (59-89): Type int256 is not implicitly convertible to expected type uint8.
// TypeError 9574: (99-127): Type int256 is not implicitly convertible to expected type uint256.
// TypeError 2271: (142-169): Binary operator == not compatible with types int256 and int_const 1157...(70 digits omitted)...9935.
