contract C {
    function(uint) external returns (uint) x;
    function(uint) internal returns (uint) y;
    function f() public {
        delete x;
        var a = y;
        delete a;
        delete y;
        var c = f;
        delete c;
        function(uint) internal returns (uint) g;
        delete g;
    }
}
