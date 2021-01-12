pragma experimental SMTChecker;

contract Reverts {
    constructor(uint) { revert("test message."); }
}
contract Succeeds {
    constructor(uint) { }
}

contract C {
    function f() public returns (Reverts x, string memory txt) {
        uint i = 3;
        try new Reverts(i) returns (Reverts r) {
            x = r;
            txt = "success";
        } catch Error(string memory s) {
            txt = s;
        }
    }
    function g() public returns (Succeeds x, string memory txt) {
        uint i = 8;
        try new Succeeds(i) returns (Succeeds r) {
            x = r;
            txt = "success";
        } catch Error(string memory s) {
            txt = s;
        }
    }
}
// ----
// Warning 4588: (264-278): Assertion checker does not yet implement this type of function call.
// Warning 4588: (525-540): Assertion checker does not yet implement this type of function call.
// Warning 4588: (264-278): Assertion checker does not yet implement this type of function call.
// Warning 4588: (525-540): Assertion checker does not yet implement this type of function call.
