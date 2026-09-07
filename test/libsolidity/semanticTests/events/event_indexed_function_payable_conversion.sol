contract C {
    event Test(function() external indexed);
    function f() public payable {}
    function g() public {
        emit Test(C(address(0x1234)).f);
    }
}
// ----
// g() ->
// ~ emit Test(function): #0x123426121ff00000000000000000
