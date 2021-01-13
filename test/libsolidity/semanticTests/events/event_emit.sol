contract ClientReceipt {
    event A(address _from, bytes32 _id, uint _value);
    event B(address _from, bytes32 _id, uint indexed _value) anonymous;
    event C(address _from, bytes32 indexed _id, uint _value);
    function deposit(bytes32 _id) public payable {
        emit A(msg.sender, _id, msg.value);
        emit B(msg.sender, _id, msg.value);
        emit C(msg.sender, _id, msg.value);
    }
}

// ====
// compileViaYul: false
// ----
// deposit(bytes32), 18 wei: 0x1234 ->
// logs.expectEvent(uint256,string): 0, "A(address,bytes32,uint256)" -> 0x1212121212121212121212121212120000000012, 0x1234, 0x12
// logs.expectEvent(uint256,string): 1, "" -> 0x12, 0x1212121212121212121212121212120000000012, 0x1234
// logs.expectEvent(uint256,string): 2, "C(address,bytes32,uint256)" -> 0x1234, 0x1212121212121212121212121212120000000012, 0x12
