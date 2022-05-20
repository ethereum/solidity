contract C {
    constructor() payable {}

    function f() public returns (uint256 ret) {
        assembly {
            ret := balance(0)
        }
    }
    function g() public returns (uint256 ret) {
        assembly {
            ret := balance(1)
        }
    }
    function h() public returns (uint256 ret) {
        assembly {
            ret := balance(address())
        }
    }
}

// ====
// EVMVersion: >=constantinople
// ----
// constructor(), 23 wei ->
// gas legacy: 100517
// f() -> 0
// g() -> 1
// h() -> 23
