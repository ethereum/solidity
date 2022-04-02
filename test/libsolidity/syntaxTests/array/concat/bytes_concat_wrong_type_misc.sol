contract C {
    struct S {
        uint x;
    }

    enum E {A, B, C}

    mapping(uint => E) m;

    function f() public {
        bool b;
        uint u;
        uint8 u8;
        address a;
        address payable ap;
        function () external fext;
        function () internal fint;
        uint[] memory uDynamic;
        uint[2] memory uStatic;
        C c;
        S memory s;
        E e;

        bytes.concat(b, u, u8, a, ap, fext, fint, uDynamic, uStatic, c, s, e, m);
    }
}
// ----
// TypeError 8015: (425-426='b'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but bool provided.
// TypeError 8015: (428-429='u'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but uint256 provided.
// TypeError 8015: (431-433='u8'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but uint8 provided.
// TypeError 8015: (435-436='a'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but address provided.
// TypeError 8015: (438-440='ap'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but address payable provided.
// TypeError 8015: (442-446='fext'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but function () external provided.
// TypeError 8015: (448-452='fint'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but function () provided.
// TypeError 8015: (454-462='uDynamic'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but uint256[] provided.
// TypeError 8015: (464-471='uStatic'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but uint256[2] provided.
// TypeError 8015: (473-474='c'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but contract C provided.
// TypeError 8015: (476-477='s'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but struct C.S provided.
// TypeError 8015: (479-480='e'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but enum C.E provided.
// TypeError 8015: (482-483='m'): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but mapping(uint256 => enum C.E) provided.
