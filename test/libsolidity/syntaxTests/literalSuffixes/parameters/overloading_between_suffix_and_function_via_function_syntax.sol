function s(uint) pure suffix returns (uint) {}
function s(string memory) pure returns (string memory) {}

function f(uint) pure returns (uint) {}
function f(string memory) pure suffix returns (string memory) {}

contract C {
    function run() public pure {
        s(1);
        s("a");

        f(1);
        f("a");
    }
}
