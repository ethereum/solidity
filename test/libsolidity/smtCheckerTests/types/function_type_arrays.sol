pragma experimental SMTChecker;
contract C {
    function(uint) external returns (uint)[] public x;
    function(uint) internal returns (uint)[10] y;
    function f() view public {
        function(uint) returns (uint)[10] memory a;
        function(uint) returns (uint)[10] storage b = y;
        function(uint) external returns (uint)[] memory c;
        c = new function(uint) external returns (uint)[](200);
        a; b;
    }
}
// ----
// Warning 4588: (361-410): Assertion checker does not yet implement this type of function call.
