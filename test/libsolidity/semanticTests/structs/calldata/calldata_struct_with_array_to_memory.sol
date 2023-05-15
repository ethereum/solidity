pragma abicoder v2;

contract C {
    struct S {
        uint256 a;
        uint256[2] b;
        uint256 c;
    }

    function f(S calldata c)
        external
        pure
        returns (uint256, uint256, uint256, uint256)
    {
        S memory m = c;
        return (m.a, m.b[0], m.b[1], m.c);
    }
}
// ----
// f((uint256,uint256[2],uint256)): 42, 1, 2, 23 -> 42, 1, 2, 23
