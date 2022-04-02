pragma abicoder               v2;
contract C {
    function f(uint256[][] calldata x) external pure {
        x[1:2];
        x[:];
        x[1:];
        x[:2];
    }
}
// ----
// TypeError 2148: (110-116='x[1:2]'): Index range access is not supported for arrays with dynamically encoded base types.
// TypeError 2148: (126-130='x[:]'): Index range access is not supported for arrays with dynamically encoded base types.
// TypeError 2148: (140-145='x[1:]'): Index range access is not supported for arrays with dynamically encoded base types.
// TypeError 2148: (155-160='x[:2]'): Index range access is not supported for arrays with dynamically encoded base types.
