contract C {
    uint256 counter = 0;

    error CustomError(uint256);

    function getCounter() public view returns (uint256) {
        return counter;
    }

    function g(bool condition) internal returns (bool) {
        counter++;
        return condition;
    }

    function f(bool condition) external {
        require(g(condition), CustomError(counter));
    }
}

// ----
// f(bool): false -> FAILURE, hex"110b3655", 1
// getCounter() -> 0
// f(bool): true ->
// getCounter() -> 1
