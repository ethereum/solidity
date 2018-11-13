contract C {
    address constant a = address(0);
    address payable constant b = address(0);
    function f() public pure returns (address, address) {
        return (a,b);
    }
}
