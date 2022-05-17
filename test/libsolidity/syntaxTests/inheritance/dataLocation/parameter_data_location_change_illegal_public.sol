abstract contract A {
    function f(uint256[1] memory a) public virtual returns (uint256);
    function test() external returns (uint) {
        uint[1] memory t;
        t[0] = 7;
        return f(t);
    }
}

contract B is A {
    function f(uint256[1] calldata a) public override returns (uint256) {
        return a[0];
    }
}
// ----
// TypeError 7723: (234-330): Data locations of parameters have to be the same when overriding non-external functions, but they differ.
