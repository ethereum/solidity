pragma abicoder v2;

contract C {
    struct S {
        uint256 a;
        uint256 b;
        bytes2 c;
    }

    function f(S calldata s) external pure returns (uint256, uint256, bytes1) {
        S memory m = s;
        return (m.a, m.b, m.c[1]);
    }
}

// ====
// compileToEwasm: also
// ----
// f((uint256,uint256,bytes2)): 42, 23, "ab" -> 42, 23, "b"
