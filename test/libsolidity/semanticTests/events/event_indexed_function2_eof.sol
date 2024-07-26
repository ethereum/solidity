contract C {
    event TestA(function() external indexed);
    event TestB(function(uint256) external indexed);
    function f1() public {
        emit TestA(this.f1);
    }
    function f2(uint256 a) public {
        emit TestB(this.f2);
    }
}
// ====
// compileToEOF: true
// EVMVersion: >=prague
// ----
// f1() ->
// ~ emit TestA(function): #0xa80a5d214a51e09e24fa3e854004da1ac3f5beffc27fc3050000000000000000
// f2(uint256): 1 ->
// ~ emit TestB(function): #0xa80a5d214a51e09e24fa3e854004da1ac3f5beffbf3724af0000000000000000
