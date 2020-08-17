pragma experimental ABIEncoderV2;

contract C {
    struct S {
        uint256 a;
        uint256 b;
        bytes2 c;
    }

    S s;

    function f(uint32 a, S calldata c, uint256 b) external returns (uint256, uint256, byte) {
        s = c;
        return (s.a, s.b, s.c[1]);
    }
}

// ====
// compileViaYul: true
// ----
// f(uint32, (uint256, uint256, bytes2), uint256): 1, 42, 23, "ab", 1 -> 42, 23, "b"