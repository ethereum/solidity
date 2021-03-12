contract C {
    address constant a = address(0);
    address payable constant b = payable(0);
    function f() public pure returns (address, address) {
        return (a,b);
    }
}
// ----
