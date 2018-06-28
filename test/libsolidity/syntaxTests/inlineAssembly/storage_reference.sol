contract C {
    uint[] x;
    function() external { 
        uint[] storage y = x;
        assembly {
            pop(y)
        }
    }
}
// ----
// TypeError: (119-120): You have to use the _slot or _offset suffix to access storage reference variables.
