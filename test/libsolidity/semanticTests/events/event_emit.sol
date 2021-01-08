contract ClientReceipt {
    event Deposit(address indexed _from, bytes32 indexed _id, uint _value);
    function deposit(bytes32 _id) public payable {
        emit Deposit(msg.sender, _id, msg.value);
    }
}
// ====
// compileViaYul: also
// ----
// deposit(bytes32), 18 wei: 0x1234 ->
// logs.numLogs() -> 1
// logs.logAddress(uint256): 0 -> 0x0fdd67305928fcac8d213d1e47bfa6165cd0b87b
// logs.logData(uint256): 0 -> 0x12
// logs.numLogTopics(uint256): 0 -> 3
// logs.logTopic(uint256,uint256): 0, 0 -> 0x19dacbf83c5de6658e14cbf7bcae5c15eca2eedecf1c66fbca928e4d351bea0f
// logs.logTopic(uint256,uint256): 0, 1 -> 0x1212121212121212121212121212120000000012
// logs.logTopic(uint256,uint256): 0, 2 -> 0x1234
