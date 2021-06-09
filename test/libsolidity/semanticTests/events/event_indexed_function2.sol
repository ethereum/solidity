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
// compileViaYul: also
// ----
// f1() ->
// ~ emit TestA(function): #0x0fdd67305928fcac8d213d1e47bfa6165cd0b87bc27fc3050000000000000000
// f2(uint256): 1 ->
// ~ emit TestB(function): #0x0fdd67305928fcac8d213d1e47bfa6165cd0b87bbf3724af0000000000000000
