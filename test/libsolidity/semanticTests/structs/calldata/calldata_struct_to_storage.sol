pragma abicoder v2;

contract C {
    struct S {
        uint256 a;
        uint64 b;
        bytes2 c;
    }

    uint[153] r;
    S s;

    function f(uint32 a, S calldata c, uint256 b) external returns (uint256, uint256, bytes1) {
        s = c;
        return (s.a, s.b, s.c[1]);
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f(uint32,(uint256,uint64,bytes2),uint256): 1, 42, 23, "ab", 1 -> 42, 23, "b"
