contract D {
    event Deposit(address indexed _from, bytes32 indexed _id, uint _value);
    function deposit(bytes32 _id) public payable {
        emit Deposit(msg.sender, _id, msg.value);
    }
}
contract C {
    D d;
    constructor() {
        d = new D();
    }
    function deposit(bytes32 _id) public payable {
        d.deposit(_id);
    }
}
// ----
// constructor() ->
// gas irOptimized: 165386
// gas legacy: 244800
// gas legacyOptimized: 171615
// deposit(bytes32), 18 wei: 0x1234 ->
// ~ emit Deposit(address,bytes32,uint256) from 0x137aa4dfc0911524504fcd4d98501f179bc13b4a: #0xc06afe3a8444fc0004668591e8306bfb9968e79e, #0x1234, 0x00
