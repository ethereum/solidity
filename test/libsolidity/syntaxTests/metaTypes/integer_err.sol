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
// TypeError: (59-89): Type int256 is not implicitly convertible to expected type uint8.
// TypeError: (99-127): Type int256 is not implicitly convertible to expected type uint256.
// TypeError: (142-169): Operator == not compatible with types int256 and int_const 1157...(70 digits omitted)...9935
