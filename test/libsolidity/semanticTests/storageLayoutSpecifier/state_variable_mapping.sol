contract A {
    mapping(uint => bool) locked;
}
contract C layout at 42 is A {
    mapping(uint => string) rooms;

    function setup() public {
        locked[0] = true;
        locked[1] = false;
        locked[2] = true;
        locked[3] = false;

        rooms[0] = "Empty";
        rooms[1] = "Monster";
        rooms[2] = "Treasure";
        rooms[3] = "Empty";
    }
    function open(uint x) public view returns (string memory) {
        if (locked[x])
            return "Locked";

        require(!locked[x]);
        return rooms[x];
    }
}
// ----
// setup() ->
// gas irOptimized: 159082
// gas legacy: 161738
// gas legacyOptimized: 160218
// gas ssaCFGOptimized: 159092
// open(uint256): 3 -> 0x20, 5, "Empty"
// open(uint256): 2 -> 0x20, 6, "Locked"
// open(uint256): 1 -> 0x20, 7, "Monster"
