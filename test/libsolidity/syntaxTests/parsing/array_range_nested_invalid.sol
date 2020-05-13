pragma experimental ABIEncoderV2;
contract C {
    function f(uint256[][] calldata x) external pure {
        x[1:2];
        x[:];
        x[1:];
        x[:2];
    }
}
// ----
// TypeError: (110-116): Index range access is not supported for arrays with dynamically encoded base types.
// TypeError: (126-130): Index range access is not supported for arrays with dynamically encoded base types.
// TypeError: (140-145): Index range access is not supported for arrays with dynamically encoded base types.
// TypeError: (155-160): Index range access is not supported for arrays with dynamically encoded base types.
