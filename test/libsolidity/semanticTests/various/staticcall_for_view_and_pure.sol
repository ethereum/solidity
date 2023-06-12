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
// gas legacy: 102095
// fview() -> FAILURE
// gas irOptimized: 98438588
// gas legacy: 98438774
// gas legacyOptimized: 98438580
// fpure() -> FAILURE
// gas irOptimized: 98438589
// gas legacy: 98438774
// gas legacyOptimized: 98438580
