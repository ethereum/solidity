contract C {
    uint[] x;
    function() external { 
        uint[] memory y = x;
        assembly {
            pop(y_slot)
            pop(y_offset)
        }
    }
}
// ----
// TypeError: (118-124): The suffixes _offset and _slot can only be used on storage variables.
// TypeError: (142-150): The suffixes _offset and _slot can only be used on storage variables.
