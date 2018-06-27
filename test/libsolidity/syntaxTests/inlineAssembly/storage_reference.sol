contract C {
    uint[] x;
    function() { 
        uint[] storage y = x;
        assembly {
            pop(y)
        }
    }
}
// ----
// TypeError: (110-111): You have to use the _slot or _offset suffix to access storage reference variables.
