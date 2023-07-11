contract C {
    function f() public returns (bytes32 ret) {
        assembly {
            ret := extcodehash(0)
        }
    }
    function g() public returns (bytes32 ret) {
        assembly {
            ret := extcodehash(1)
        }
    }
    function h() public returns (bool ret) {
        assembly {
            ret := iszero(iszero(extcodehash(address())))
        }
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// f() -> 0
// g() -> 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470
// h() -> true
