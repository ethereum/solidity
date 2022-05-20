pragma solidity >= 0.6.0;

contract C {
    function g(uint n) external pure returns (uint) {
        return n + 1;
    }

    function f(uint n) public view returns (uint) {
        return this.g(2 * n);
    }
}

// ----
// g(uint256): 4 -> 5
// f(uint256): 2 -> 5
