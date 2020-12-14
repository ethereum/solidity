contract C {
    function f() public view {
        address a1 = address(uint160(0));
        address a2 = address(bytes20(0));
        address a3 = address(this);

        address payable a4 = payable(uint160(0));
        address payable a5 = payable(bytes20(0));
        address payable a6 = payable(this);

        // Trivial conversions
        address payable a7 = payable(address(uint160(0)));
        address payable a8 = payable(address(bytes20(0)));
        address payable a9 = payable(address(this));

        a1; a2; a3; a4; a5; a6; a7; a8; a9;
    }

    // to make payable(this) work
    receive() payable external {
    }
}
// ----
