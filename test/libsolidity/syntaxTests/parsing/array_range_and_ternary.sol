contract C {
    function f(bool cond, bytes calldata x) external pure {
        bytes1 a = x[cond ? 1 : 2]; a;
        abi.decode(x[cond ? 1 : 2 : ], (uint256));
        abi.decode(x[cond ? 1 : 2 : cond ? 3 : 4], (uint256));
    }
}
// ----
