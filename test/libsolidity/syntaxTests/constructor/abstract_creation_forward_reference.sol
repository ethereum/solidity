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
// TypeError 3656: (226-254): Contract "Child" should be marked as abstract.
