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

// ====
// compileViaYul: also
// ----
// f() -> 1
// gas irOptimized: 111246
// gas legacy: 114412
// g() -> 5
// gas irOptimized: 111379
// gas legacy: 114872
