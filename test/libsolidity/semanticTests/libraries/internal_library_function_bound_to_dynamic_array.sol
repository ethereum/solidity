library L {
    function at(uint[] memory a, uint i) internal pure returns (uint) {
        return a[i];
    }
}

contract C {
    using L for uint[];

    function secondItem() public returns (uint) {
        uint[] memory input = new uint[](2);
        input[0] = 0x11;
        input[1] = 0x22;

        return input.at(1);
    }
}
// ----
// secondItem() -> 0x22
