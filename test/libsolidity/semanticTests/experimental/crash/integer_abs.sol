pragma experimental solidity;

contract C {
    fallback() external {
        integer.abs();
    }
}
// ----
// () ->
