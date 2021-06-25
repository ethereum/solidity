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
            0x11112222333344445555666677778888999900,     // One byte less than an address
            0x1111222233334444555566667777888899990000,   // Address
            0x111122223333444455556666777788889999000011, // One byte more than an address
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
// TypeError 1227: (540-546): Index range access is only supported for dynamic calldata arrays.
// TypeError 8015: (133-138): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but bool provided.
// TypeError 8015: (152-153): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 1 provided.
// TypeError 8015: (167-171): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 10000000000 provided.
// TypeError 8015: (185-190): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but rational_const 1 / 10000000000 provided.
// TypeError 8015: (204-207): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but rational_const 1 / 10 provided.
// TypeError 8015: (221-261): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 3806...(37 digits omitted)...1680 provided.
// TypeError 8015: (312-354): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but address provided.
// TypeError 8015: (381-425): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but int_const 2494...(42 digits omitted)...0497 provided.
// TypeError 8015: (472-473): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but function () provided.
// TypeError 8015: (487-489): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but tuple() provided.
// TypeError 8015: (503-509): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but tuple(int_const 0,int_const 0) provided.
// TypeError 8015: (523-526): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but uint8[1] provided.
// TypeError 8015: (540-546): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but uint8[1] slice provided.
// TypeError 8015: (560-566): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but uint8 provided.
// TypeError 8015: (580-587): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but contract C provided.
// TypeError 8015: (601-605): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but struct C.S provided.
// TypeError 8015: (619-622): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but enum C.E provided.
