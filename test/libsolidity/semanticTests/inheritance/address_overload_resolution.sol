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
// gas ir: 172056
// gas irOptimized: 111246
// gas legacy: 132436
// gas legacyOptimized: 100628
// g() -> 5
// gas ir: 172294
// gas irOptimized: 111379
// gas legacy: 132896
// gas legacyOptimized: 100753
