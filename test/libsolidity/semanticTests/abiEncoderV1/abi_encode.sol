contract C {
    function f0() public returns (bytes memory) {
        return abi.encode();
    }

    function f1() public returns (bytes memory) {
        return abi.encode(1, 2);
    }

    function f2() public returns (bytes memory) {
        string memory x = "abc";
        return abi.encode(1, x, 2);
    }

    function f3() public returns (bytes memory r) {
        // test that memory is properly allocated
        string memory x = "abc";
        r = abi.encode(1, x, 2);
        bytes memory y = "def";
        require(y[0] == "d");
        y[0] = "e";
        require(y[0] == "e");
    }

    function f4() public returns (bytes memory) {
        bytes4 x = "abcd";
        return abi.encode(bytes2(x));
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f0() -> 0x20, 0x0
// f1() -> 0x20, 0x40, 0x1, 0x2
// f2() -> 0x20, 0xa0, 0x1, 0x60, 0x2, 0x3, "abc"
// f3() -> 0x20, 0xa0, 0x1, 0x60, 0x2, 0x3, "abc"
// f4() -> 0x20, 0x20, "ab"
