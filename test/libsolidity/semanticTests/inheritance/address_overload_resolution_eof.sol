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
// compileToEOF: true
// EVMVersion: >=prague
// ----
// f() -> 1
// gas irOptimized: 77051
// gas legacy: 54480
// gas legacy code: 57800
// g() -> 5
// gas irOptimized: 77106
// gas legacy: 55016
// gas legacy code: 57800
