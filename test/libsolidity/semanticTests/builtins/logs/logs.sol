contract ClientReceipt {
    event A(address _from, bytes32 _id, uint _value);
    event B(address _from, bytes32 _id, uint indexed _value) anonymous;
    event C(address _from, bytes32 indexed _id, uint _value);
    function deposit(bytes32 _id) public payable returns(uint256) {
        emit A(msg.sender, _id, msg.value);
        emit B(msg.sender, _id, msg.value);
        emit C(msg.sender, _id, msg.value);
        return  msg.value;
    }
}

// ====
// compileViaYul: false
// ----
// deposit(bytes32), 28 wei: 0x1234 -> 0x1c
// logs.numLogTopics: 0 -> 1
// logs.numLogTopics: 1 -> 1
// logs.numLogTopics: 2 -> 2
// deposit(bytes32), 28 wei: 0x1234 -> 0x1c
// logs.numLogTopics: 0 -> 1
// logs.numLogTopics: 1 -> 1
// logs.numLogTopics: 2 -> 2
// deposit(bytes32), 28 wei: 0x1234 -> 0x1c
// logs.numLogTopics: 0 -> 1
// logs.numLogTopics: 1 -> 1
// logs.numLogTopics: 2 -> 2
