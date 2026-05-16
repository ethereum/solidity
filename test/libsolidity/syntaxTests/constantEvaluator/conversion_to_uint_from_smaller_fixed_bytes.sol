bytes16 constant B16 = hex"00112233445566778899aabbccddeeff";
bytes8 constant B8 = "12345678";

contract C {
    int[uint128(B16)] arr1;
    int[uint64(B8)] arr2;
}
// ----
// TypeError 5462: (117-129): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (145-155): Invalid array length, expected integer literal or constant expression.
