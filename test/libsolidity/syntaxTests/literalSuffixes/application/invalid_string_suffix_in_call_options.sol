function suffix(uint) pure suffix returns (string memory) {}

interface I {
    function f() external payable;
}
contract C {
    function f() public {
        I(address(42)).f{value: 5 suffix, gas: 1 suffix}();
    }
}
// ----
// TypeError 7407: (184-192): Type string memory is not implicitly convertible to expected type uint256.
// TypeError 7407: (199-207): Type string memory is not implicitly convertible to expected type uint256.
