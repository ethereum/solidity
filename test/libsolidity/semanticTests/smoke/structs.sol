pragma experimental ABIEncoderV2;

contract C {
    struct S {
        uint a;
        uint b;
    }
    struct T {
        uint a;
        uint b;
        string s;
    }
    function s() public returns (S memory) {
        return S(23, 42);
    }
    function t() public returns (T memory) {
        return T(23, 42, "any");
    }
}
// ----
// s() -> 23, 42
// t() -> 0x20, 23, 42, 0x60, 3, "any"
