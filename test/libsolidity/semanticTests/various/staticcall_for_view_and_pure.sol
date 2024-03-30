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
// ----
// f() -> 0x1 # This should work, next should throw #
// gas legacy: 76495
// gas legacy code: 25600
// fview() -> FAILURE
// gas irOptimized: 98425388
// gas irOptimized code: 13200
// gas legacy: 98413173
// gas legacy code: 25600
// gas legacyOptimized: 98425379
// gas legacyOptimized code: 13200
// fpure() -> FAILURE
// gas irOptimized: 98425388
// gas irOptimized code: 13200
// gas legacy: 98413173
// gas legacy code: 25600
// gas legacyOptimized: 98425379
// gas legacyOptimized code: 13200
