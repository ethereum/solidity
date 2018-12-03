// This used to cause an internal error because of the visitation order.
contract Test {
    function createChild() public {
       Child asset = new Child();
    }
}

contract Parent {
    constructor(address _address) public {}
}

contract Child is Parent {
}
// ----
// TypeError: (146-155): Trying to create an instance of an abstract contract.
