library L {
    function double(uint a) internal pure returns (uint) {
        return a * 2;
    }

    function double(bytes memory a) internal pure returns (bytes memory) {
        return bytes.concat(a, a);
    }
}

contract C {
    using L for *;

    function double42() public returns (uint) {
        return 42.double();
    }

    function doubleABC() public returns (bytes memory) {
        return "abc".double();
    }
}
// ----
// double42() -> 84
// doubleABC() -> 0x20, 6, "abcabc"
