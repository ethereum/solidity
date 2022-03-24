abstract contract A {
    function f(uint256[1] memory a) external virtual returns (uint256);
}
contract B is A {
    function f(uint256[1] calldata a) external pure virtual override returns (uint256) {
        return a[0];
    }
}
contract C is A, B {
    function f(uint256[1] memory a) external pure override(B, A) returns (uint256) {
        return a[0];
    }
}
// ----
