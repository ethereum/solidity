pragma experimental solidity;

contract C {
    fallback() external {
        let x: bool;

        if (x) {}
        else {}
    }
}
// ----
// (): ->
