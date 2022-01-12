contract Reverts {
    constructor(uint) { revert("test message."); }
}
contract Succeeds {
    constructor(uint) { }
}

contract C {
    function f() public returns (Reverts x, uint, string memory txt) {
        uint i = 3;
        try new Reverts(i) returns (Reverts r) {
            x = r;
            txt = "success";
        } catch Error(string memory s) {
            txt = s;
        }
    }
    function g() public returns (Succeeds x, uint, string memory txt) {
        uint i = 8;
        try new Succeeds(i) returns (Succeeds r) {
            x = r;
            txt = "success";
        } catch Error(string memory s) {
            txt = s;
        }
    }
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// ----
// f() -> 0, 0, 0x60, 13, 52647538820800208530922410608562059381415374016670515336697209233794224095232
// g() -> 1370859564726510389319704988634906228201275401179, 0, 0x60, 7, 0x7375636365737300000000000000000000000000000000000000000000000000
