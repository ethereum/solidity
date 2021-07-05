interface I {
    function f(uint256[] calldata a) external returns (uint256);
}


contract A is I {
    function f(uint256[] calldata a) external override returns (uint256) {
        return 42;
    }
}


contract B {
    function f(uint256[] memory a) public returns (uint256) {
        return a[1];
    }

    function g() public returns (uint256) {
        I i = I(new A());
        return i.f(new uint256[](2));
    }
}

// ====
// compileViaYul: also
// ----
// g() -> 42
// gas irOptimized: 80942
// gas legacy: 125609
