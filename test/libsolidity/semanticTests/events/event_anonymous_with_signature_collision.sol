contract ClientReceipt {
    event Deposit(uint256 indexed _from, bytes32 indexed _id, uint _value) anonymous;
    function deposit(bytes32 _id) public payable {
        emit Deposit(0x2012159ca6b6372f102c535a4814d13a00bfc5568ddfd72151364061b00355d1, _id, msg.value); // 0x2012159c -> 'Deposit(uint256,bytes32,uint256)'
    }
}
// ----
// deposit(bytes32), 18 wei: 0x1234 ->
// ~ emit <anonymous>: #0x2012159ca6b6372f102c535a4814d13a00bfc5568ddfd72151364061b00355d1, #0x1234, 0x12
