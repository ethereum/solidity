contract C {
    function f() public view returns (address payable a, address b) {
        (address c, address payable d) = (address(this), address(0));
        (a,b) = (d,c);
    }
}