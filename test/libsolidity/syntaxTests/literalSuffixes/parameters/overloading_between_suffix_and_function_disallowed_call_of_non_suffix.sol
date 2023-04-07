function s(uint) pure suffix returns (uint) {}
function s(string memory) pure returns (string memory) {}

contract C {
    function run() public pure {
        "a" s;
    }
}
// ----
// TypeError 2144: (164-165): No matching declaration found after variable lookup.
