pragma abicoder               v2;


// Checks that bytesXX types are properly cleaned before they are compared.
contract C {
    function f(bytes2 a, uint16 x) public returns (uint256) {
        if (a != "ab") return 1;
        if (x != 0x0102) return 2;
        if (bytes3(uint24(x)) != 0x000102) return 3;
        return 0;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(bytes2,uint16): "abc", 0x40102 -> FAILURE # We input longer data on purpose. #
