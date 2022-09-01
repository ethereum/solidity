contract ClientReceipt {
    event Deposit(address _from, bytes32 _id, uint _value, bool _flag);
    function deposit(bytes32 _id) public payable {
        emit Deposit(msg.sender, _id, msg.value, true);
    }
}
// ----
// deposit(bytes32), 18 wei: 0x1234 ->
// ~ emit Deposit(address,bytes32,uint256,bool): 0x1212121212121212121212121212120000000012, 0x1234, 0x12, true
