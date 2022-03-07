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
// ====
// compileViaYul: also
// ----
// constructor() ->
// gas irOptimized: 172802
// gas legacy: 250376
// gas legacyOptimized: 174522
// deposit(bytes32), 18 wei: 0x1234 ->
// ~ emit Deposit(address,bytes32,uint256) from 0xf01f7809444bd9a93a854361c6fae3f23d9e23db: #0x0fdd67305928fcac8d213d1e47bfa6165cd0b87b, #0x1234, 0x00
