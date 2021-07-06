contract ClientReceipt {
    event Deposit(address indexed _from, bytes32 indexed _id, uint _value);
    function deposit(bytes32 _id, bool _manually) public payable {
        if (_manually) {
            bytes32 s = 0x19dacbf83c5de6658e14cbf7bcae5c15eca2eedecf1c66fbca928e4d351bea0f;
            uint value = msg.value;
            address sender = msg.sender;
            assembly {
                mstore(0, value)
                log3(0, 0x20, s, sender, _id)
            }
        } else {
            emit Deposit(msg.sender, _id, msg.value);
        }
    }
}
// ====
// compileViaYul: also
// ----
// deposit(bytes32,bool), 18 wei: 0x1234, true ->
// ~ emit Deposit(address,bytes32,uint256): #0x1212121212121212121212121212120000000012, #0x1234, 0x12
// deposit(bytes32,bool), 18 wei: 0x1234, false ->
// ~ emit Deposit(address,bytes32,uint256): #0x1212121212121212121212121212120000000012, #0x1234, 0x12
