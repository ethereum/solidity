pragma experimental solidity;

contract C {
    fallback() external {
        revert;
    }
}
// ----
// () ->
