contract ClientReceipt {
    event D(address indexed _from, bytes32 indexed _id, uint _value);
    event D2(address indexed _from, bytes32 indexed _id, uint _value) anonymous;
    event D3(address _from, bytes32 indexed _id, uint _value);
    function deposit(bytes32 _id) public payable {
        emit D(msg.sender, _id, msg.value);
        emit D2(msg.sender, _id, msg.value);
        emit D3(msg.sender, _id, msg.value);
    }
    function deposit2(bytes32 _id) public payable {
        emit D(msg.sender, _id, msg.value);
    }
}

// logs.expectEvent(uint256,string): 0, "D(address,bytes32,uint256)" -> 0x1212121212121212121212121212120000000012, 0x1234, 18

// deposit2(bytes32), 18 wei: 0x1234 ->
// deposit(bytes32), 18 wei: 0x1234 ->
// ====
// compileViaYul: false
// ----
// deposit(bytes32), 18 wei: 0x1234 ->
// logs.expectEvent(uint256,string): 0, "D(address,bytes32,uint256)" -> 0x1212121212121212121212121212120000000012, 0x1234, 18
// logs.expectEvent(uint256,string): 1, "D2(address,bytes32,uint256)" -> 0x1212121212121212121212121212120000000012, 0x1234, 18
// logs.expectEvent(uint256,string): 2, "D3(address,bytes32,uint256)" -> 0x1234, 0x1212121212121212121212121212120000000012, 0x12
