contract C {
    function f() public pure {
        bytes.concat(
            0,
            -0,
            0.0,
            -0.0,
            0e10,
            -0e10,
            0e-10,
            -0e-10,
            (0),
            0x00,
            -0x00,
            0x0000000000000000000000000000000000000000000000000000000000000000, // exactly 32 bytes
            -0x0000000000000000000000000000000000000000000000000000000000000000 // exactly 32 bytes
        );
    }
}
// ----
// TypeError 8015: (78-79='0'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (93-95='-0'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (109-112='0.0'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (126-130='-0.0'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (144-148='0e10'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (162-167='-0e10'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (181-186='0e-10'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (200-206='-0e-10'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (220-223='(0)'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (237-241='0x00'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (255-260='-0x00'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (274-340): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (374-441): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
