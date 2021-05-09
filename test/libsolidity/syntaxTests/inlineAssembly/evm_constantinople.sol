contract C {
    function f() view external returns (uint ret) {
        assembly {
            ret := shl(gas(), 5)
            ret := shr(ret, 2)
            ret := sar(ret, 2)
        }
    }
    function g() external returns (address ret) {
        assembly {
            ret := create2(0, 0, 0, 0)
        }
    }
    function h() view external returns (bytes32 ret) {
        assembly {
            ret := extcodehash(address())
        }
    }
}
// ====
// EVMVersion: >=constantinople
// ----
