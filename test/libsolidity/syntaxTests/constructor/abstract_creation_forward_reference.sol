// This used to cause an internal error because of the visitation order.
contract Test {
    function createChild() public {
       Child asset = new Child();
    }
}

contract Parent {
    constructor(address _address) {}
}

contract Child is Parent {
}
// ----
// TypeError 3415: (226-254): No arguments passed to the base constructor. Specify the arguments or mark "Child" as abstract.
