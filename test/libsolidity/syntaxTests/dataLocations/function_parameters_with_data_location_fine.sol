contract C {
    function f(uint[] memory, uint[] storage) private pure {}
    function g(uint[] memory, uint[] storage) internal pure {}
    function h(uint[] memory) public pure {}
    function i(uint[] calldata) external pure {}
    // No data location for events.
    event e(uint[]);
}
