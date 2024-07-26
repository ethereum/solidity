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
// compileToEOF: true
// EVMVersion: >=prague
// ----
// constructor() ->
// gas irOptimized: 113970
// gas irOptimized code: 51400
// gas legacy: 119776
// gas legacy code: 125000
// gas legacyOptimized: 114187
// gas legacyOptimized code: 57400
// deposit(bytes32), 18 wei: 0x1234 ->
// ~ emit Deposit(address,bytes32,uint256) from 0x32b73100436177e8f2d2aa1214bb4c1230143ec2: #0x3cb69f8aa1103d7ce41821a1b2e1c85c2b63dfa4, #0x1234, 0x00
