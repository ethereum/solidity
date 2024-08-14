contract C {
    event Test(function() external indexed);
    function f() public {
        emit Test(this.f);
    }
}
// ====
// compileToEOF: true
// EVMVersion: >=prague
// ----
// f() ->
// ~ emit Test(function): #0x1141c91a4a817b60b5339bc09ac809cedc7649ab26121ff00000000000000000
