contract C {
    function f(uint[] storage x) private {
    }
    function g(uint[] x) public {
        f(x);
    }
}
// ----
// TypeError: (106-107): Invalid type for argument in function call. Invalid implicit conversion from uint256[] memory to uint256[] storage pointer requested.
