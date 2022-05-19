pragma abicoder v2;

contract C {
    struct S {
        uint64 a;
        bytes b;
    }
    struct S1 {
        uint256 a;
        S s;
        uint256 c;
    }

    function f(S1 calldata s1)
        external
        pure
        returns (uint256 a, uint64 b0, bytes1 b1, uint256 c)
    {
        a = s1.a;
        b0 = s1.s.a;
        b1 = s1.s.b[0];
        c = s1.c;
    }
}
// ----
// f((uint256,(uint64,bytes),uint256)): 0x20, 42, 0x60, 23, 1, 0x40, 2, "ab" -> 42, 1, "a", 23
