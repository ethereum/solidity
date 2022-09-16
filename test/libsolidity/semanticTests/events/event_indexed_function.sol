contract C {
    event Test(function() external indexed);
    function f() public {
        emit Test(this.f);
    }
}
// ----
// f() ->
// ~ emit Test(function): #0xc06afe3a8444fc0004668591e8306bfb9968e79e26121ff00000000000000000
