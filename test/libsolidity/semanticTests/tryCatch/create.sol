contract Reverts {
    constructor(uint) { revert("test message."); }
}
contract Succeeds {
    constructor(uint) { }
}

contract C {
    function f() public returns (bool created, string memory txt) {
        uint i = 3;
        try new Reverts(i) returns (Reverts r) {
            created = (address(r) != address(0));
            txt = "success";
        } catch Error(string memory s) {
            txt = s;
        }
    }
    function g() public returns (bool created, string memory txt) {
        uint i = 8;
        try new Succeeds(i) returns (Succeeds r) {
            created = (address(r) != address(0));
            txt = "success";
        } catch Error(string memory s) {
            txt = s;
        }
    }
}
// ====
// EVMVersion: >=byzantium
// targetContract: C
// ----
// f() -> false, 0x40, 13, "test message."
// g() -> true, 0x40, 7, "success"
