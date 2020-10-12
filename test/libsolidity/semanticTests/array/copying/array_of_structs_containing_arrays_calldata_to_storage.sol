pragma experimental ABIEncoderV2;

contract C {
    struct S {
        uint256[] a;
    }

    S[] s;

    function f(S[] calldata c) external returns (uint256, uint256) {
        s = c;
        return (s[1].a.length, s[1].a[0]);
    }
}
// ====
// compileViaYul: true
// ----
// f((uint256[])[]): 0x20, 3, 0x60, 0x60, 0x60, 0x20, 3, 1, 2, 3 -> 3, 1