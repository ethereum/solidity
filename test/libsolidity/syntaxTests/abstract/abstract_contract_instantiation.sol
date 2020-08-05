abstract contract AbstractContract {
    constructor() { }
    function utterance() public returns (bytes32) { return "miaow"; }
}

contract Test {
    function create() public {
       AbstractContract ac = new AbstractContract();
    }
}
// ----
// TypeError 4614: (208-228): Cannot instantiate an abstract contract.
