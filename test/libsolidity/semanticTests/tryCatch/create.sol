contract Reverts {
    constructor(uint) public { revert("test message."); }
}
contract Succeeds {
    constructor(uint) public { }
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
// ----
// f() -> 0, 0, 96, 13, "test message."
// g() -> 0xf01f7809444bd9a93a854361c6fae3f23d9e23db, 0, 96, 7, "success"
