type T is uint8;

contract C {
    struct S { T x; }
    type T is bytes32;

    function f() public pure returns (bytes32) {
        S memory s = S(T.wrap(bytes32(type(uint).max)));
        return T.unwrap(s.x);
    }
}
// ----
// f() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
