// The IR of this test used to throw
contract D {
    constructor(function() external returns (uint)) {
    }
}
// ====
// compileViaYul: also
// ----
// constructor(): 0xfdd67305928fcac8d213d1e47bfa6165cd0b87b946644cd0000000000000000 ->
