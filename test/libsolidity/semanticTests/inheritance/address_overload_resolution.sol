contract C {
    function balance() public returns (uint256) {
        return 1;
    }

    function transfer(uint256 amount) public returns (uint256) {
        return amount;
    }
}


contract D {
    function f() public returns (uint256) {
        return (new C{salt: hex"00"}()).balance();
    }

    function g() public returns (uint256) {
        return (new C{salt: hex"01"}()).transfer(5);
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// f() -> 1
// gas irOptimized: 54008
// gas irOptimized code: 20000
// gas legacy: 54553
// gas legacy code: 57800
// g() -> 5
// gas irOptimized: 54036
// gas irOptimized code: 20000
// gas legacy: 55090
// gas legacy code: 57800
