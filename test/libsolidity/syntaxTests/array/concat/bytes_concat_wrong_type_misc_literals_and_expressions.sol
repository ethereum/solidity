contract C {
    struct S {
        uint x;
    }

    enum E {A, B, C}

    function f() public {
        bytes.concat(
            false,
            1,
            1e10,
            1e-10,
            0.1,
            0x1234567,
            0x11112222333344445555666677778888999900,     // One byte less than an address
            0x1111222233334444555566667777888899990000,   // Address
            0x111122223333444455556666777788889999000011, // One byte more than an address
            0x00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff,  // exactly 32 bytes
            -0x0000000000000000000000000000000000000000000000000000000000000001, // exactly 32 bytes
            bytes(bytes32(0x00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff))[:],
            f,
            (),
            (0, 0),
            [0],
            [0][:],
            [0][0],
            new C(),
            S(0),
            E.A
        );
    }
}
// ----
// TypeError 9640: (697-779): Explicit type conversion not allowed from "bytes32" to "bytes memory".
// TypeError 1227: (697-782): Index range access is only supported for dynamic calldata arrays.
// TypeError 1227: (864-870): Index range access is only supported for dynamic calldata arrays.
// TypeError 8015: (133-138): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but bool provided.
// TypeError 8015: (152-153): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 1 provided.
// TypeError 8015: (167-171): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 10000000000 provided.
// TypeError 8015: (185-190): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but rational_const 1 / 10000000000 provided.
// TypeError 8015: (204-207): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but rational_const 1 / 10 provided.
// TypeError 8015: (221-230): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 19088743 provided.
// TypeError 8015: (244-284): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 3806...(37 digits omitted)...1680 provided.
// TypeError 8015: (335-377): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but address provided.
// TypeError 8015: (404-448): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 2494...(42 digits omitted)...0497 provided.
// TypeError 8015: (495-561): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 3027...(66 digits omitted)...5855 provided.
// TypeError 8015: (596-663): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const -1 provided.
// TypeError 8015: (697-782): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but bytes memory slice provided.
// TypeError 8015: (796-797): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but function () provided.
// TypeError 8015: (811-813): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but tuple() provided.
// TypeError 8015: (827-833): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but tuple(int_const 0,int_const 0) provided.
// TypeError 8015: (847-850): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but uint8[1] memory provided.
// TypeError 8015: (864-870): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but uint8[1] memory slice provided.
// TypeError 8015: (884-890): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but uint8 provided.
// TypeError 8015: (904-911): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but contract C provided.
// TypeError 8015: (925-929): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but struct C.S memory provided.
// TypeError 8015: (943-946): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but enum C.E provided.
