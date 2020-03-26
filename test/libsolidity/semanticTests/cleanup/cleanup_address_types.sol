// Checks that address types are properly cleaned before they are compared.
contract C {
    function f(address a) public returns (uint256) {
        if (a != 0x1234567890123456789012345678901234567890) return 1;
        return 0;
    }

    function g(address payable a) public returns (uint256) {
        if (a != 0x1234567890123456789012345678901234567890) return 1;
        return 0;
    }
}
// ====
// ABIEncoderV1Only: true
// ----
// f(address): 0xffff1234567890123456789012345678901234567890 -> 0x0 # We input longer data on purpose.#
// g(address): 0xffff1234567890123456789012345678901234567890 -> 0x0
