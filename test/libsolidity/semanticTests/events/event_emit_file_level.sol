event Deposit(address indexed _from, bytes32 indexed _id, uint _value);

contract ClientReceipt {
    function deposit(bytes32 _id) public payable {
        emit Deposit(msg.sender, _id, msg.value);
    }
}
// ----
// deposit(bytes32), 18 wei: 0x1234 ->
// ~ emit Deposit(address,bytes32,uint256): #0x1212121212121212121212121212120000000012, #0x1234, 0x12
