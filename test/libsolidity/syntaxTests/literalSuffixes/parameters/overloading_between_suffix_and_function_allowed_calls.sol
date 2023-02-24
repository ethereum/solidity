function s(uint) pure suffix returns (uint) {}
function s(string memory) pure returns (string memory) {}

function f(uint) pure returns (uint) {}
function f(string memory) pure suffix returns (string memory) {}

contract C {
    function run() public pure {
        1 s;
        s(1);
        //"a" s;  // not allowed
        s("a");

        //1 f;    // not allowed
        f(1);
        "a" f;
        f("a");
    }
}
