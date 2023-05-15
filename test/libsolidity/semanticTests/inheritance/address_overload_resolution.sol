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
        return (new C()).balance();
    }

    function g() public returns (uint256) {
        return (new C()).transfer(5);
    }
}
// ----
// f() -> 1
// gas irOptimized: 77051
// gas legacy: 112280
// g() -> 5
// gas irOptimized: 77106
// gas legacy: 112816
