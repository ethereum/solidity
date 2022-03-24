abstract contract A {
    function f(uint256[1] calldata a) public virtual returns (uint256);
}

contract B is A {
    function f(uint256[1] memory a) public override returns (uint256) {
        return a[0];
    }
}
// ----
// TypeError 7723: (119-213): Data locations of parameters have to be the same when overriding non-external functions, but they differ.
