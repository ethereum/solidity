abstract contract AbstractContract {
    constructor() public { }
    function utterance() public returns (bytes32) { return "miaow"; }
}

contract Test {
    function create() public {
       AbstractContract ac = new AbstractContract();
    }
}
// ----
// TypeError: (215-235): Cannot instantiate an abstract contract.
