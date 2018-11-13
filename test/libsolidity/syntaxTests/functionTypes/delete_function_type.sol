contract C {
    function(uint) external returns (uint) x;
    function(uint) internal returns (uint) y;
    function f() public {
        delete x;
        function(uint) internal returns (uint) a = y;
        delete a;
        delete y;
        function() internal c = f;
        delete c;
        function(uint) internal returns (uint) g;
        delete g;
    }
}
// ----
