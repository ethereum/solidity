contract C {
    uint256 x;

    function f() public returns (uint256) {
        x = 3;
        return 1;
    }
}


interface CView {
    function f() external view returns (uint256);
}


interface CPure {
    function f() external pure returns (uint256);
}


contract D {
    function f() public returns (uint256) {
        return (new C()).f();
    }

    function fview() public returns (uint256) {
        return (CView(address(new C()))).f();
    }

    function fpure() public returns (uint256) {
        return (CPure(address(new C()))).f();
    }
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// ----
// f() -> 0x1 # This should work, next should throw #
// gas ir: 186425
// gas legacy: 120980
// gas legacyOptimized: 111388
// fview() -> FAILURE
// gas ir: 98440114
// gas irOptimized: 98438674
// gas legacy: 98439103
// gas legacyOptimized: 98438960
// fpure() -> FAILURE
// gas ir: 98440114
// gas irOptimized: 98438674
// gas legacy: 98439104
// gas legacyOptimized: 98438960
