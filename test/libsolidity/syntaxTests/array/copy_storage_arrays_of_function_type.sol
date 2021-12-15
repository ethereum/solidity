contract C {
    function() external [] s0;
    function() external view [] s1;
    function copyStorageArrayOfFunctionType() public {
        s1 = s0;
    }
}
// ----
// TypeError 7407: (148-150): Type function () external[] storage ref is not implicitly convertible to expected type function () view external[] storage ref.
