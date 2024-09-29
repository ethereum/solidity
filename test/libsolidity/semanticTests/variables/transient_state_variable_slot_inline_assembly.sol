contract C {
    uint256 y;
    uint256 transient x;
    int8 transient w;
    int z;
    address transient a;
    function f() public returns(uint256 s, uint256 o) {
        assembly {
            s := x.slot
            o := x.offset
        }
    }
    function g() public returns(uint256 s, uint256 o) {
        assembly {
            s := w.slot
            o := w.offset
        }
    }
    function h() public returns(uint256 s, uint256 o) {
        assembly {
            s := a.slot
            o := a.offset
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f() -> 0, 0
// g() -> 1, 0
// h() -> 1, 1
