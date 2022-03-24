abstract contract A {
    function f(uint256[1] memory a) internal virtual returns (uint256);
    function test() external returns (uint) {
        uint[1] memory t;
        t[0] = 7;
        return f(t);
    }
}

contract B is A {
    function f(uint256[1] calldata a) internal override returns (uint256) {
        return a[0];
    }
}
// ----
// TypeError 7723: (236-334): Data locations of parameters have to be the same when overriding non-external functions, but they differ.
