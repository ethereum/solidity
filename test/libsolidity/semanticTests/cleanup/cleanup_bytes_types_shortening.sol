contract C {
    function f() public pure returns (bytes32 r) {
        bytes4 x = 0xffffffff;
        bytes2 y = bytes2(x);
        assembly {
            r := y
        }
        // At this point, r and y both store four bytes, but
        // y is properly cleaned before the equality check
        require(y == bytes2(0xffff));
    }
}
// ====
// compileViaYul: also
// ----
// f() -> "\xff\xff\xff\xff"
