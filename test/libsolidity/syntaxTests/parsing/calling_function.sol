contract test {
    function f() public {
        function() returns(function() returns(function() returns(function() returns(uint)))) x;
        uint y;
        y = x()()()();
    }
}
