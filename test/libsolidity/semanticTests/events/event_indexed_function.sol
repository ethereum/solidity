contract C {
    event Test(function() external indexed);
    function f() public {
        emit Test(this.f);
    }
}
// ====
// compileViaYul: also
// ----
// f() ->
// ~ emit Test(function): #0x0fdd67305928fcac8d213d1e47bfa6165cd0b87b26121ff00000000000000000
