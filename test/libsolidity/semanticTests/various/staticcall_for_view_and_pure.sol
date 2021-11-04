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
// gas legacy: 103716
// fview() -> FAILURE
// gas irOptimized: 98438625
// gas legacy: 98438801
// gas legacyOptimized: 98438594
// fpure() -> FAILURE
// gas irOptimized: 98438626
// gas legacy: 98438801
// gas legacyOptimized: 98438595
