library L {
    function at(uint[2] memory a, uint i) internal pure returns (uint) {
        return a[i];
    }
}

contract C {
    using L for uint[2];

    function secondItem() public returns (uint) {
        uint[2] memory input;
        input[0] = 0x11;
        input[1] = 0x22;

        return input.at(1);
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// secondItem() -> 0x22
