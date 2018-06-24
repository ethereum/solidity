contract test {
    function f() {
        function() returns(function() returns(function() returns(function() returns(uint)))) x;
        uint y;
        y = x()()()();
    }
}
// ----
// Warning: (20-175): No visibility specified. Defaulting to "public". 
