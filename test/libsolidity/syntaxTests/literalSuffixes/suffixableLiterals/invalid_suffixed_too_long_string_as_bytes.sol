function suffix(bytes2) pure suffix returns (uint) {}

contract C {
    uint x = "abcd" suffix;
    uint y = unicode"abcd" suffix;
    uint z = hex"12345678" suffix;
}
// ----
// TypeError 8838: (81-87): The literal cannot be converted to type bytes2 accepted by the suffix function.
// TypeError 8838: (109-122): The literal cannot be converted to type bytes2 accepted by the suffix function.
// TypeError 8838: (144-157): The literal cannot be converted to type bytes2 accepted by the suffix function.
