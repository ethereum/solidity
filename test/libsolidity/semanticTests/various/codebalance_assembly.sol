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
// compileViaYul: also
// ----
// constructor(), 23 wei ->
// gas ir: 172463
// gas irOptimized: 125972
// gas legacy: 119967
// gas legacyOptimized: 94295
// f() -> 0
// g() -> 1
// h() -> 23
