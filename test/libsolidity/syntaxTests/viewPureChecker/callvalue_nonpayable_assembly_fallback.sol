contract C
{
    function () external {
        uint x;
        assembly {
            x := callvalue()
        }
    }
}
// ----
// TypeError: (92-103): "msg.value" and "callvalue()" can only be used in payable public functions. Make the function "payable" or use an internal function to avoid this error.
