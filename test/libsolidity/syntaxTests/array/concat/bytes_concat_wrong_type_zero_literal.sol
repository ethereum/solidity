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
// TypeError 8015: (78-79): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (93-95): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (109-112): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (126-130): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (144-148): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (162-167): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (181-186): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (200-206): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (220-223): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (237-241): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (255-260): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (274-340): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
// TypeError 8015: (374-441): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 0 provided.
