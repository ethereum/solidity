contract C
{
    function f(uint x) public {
        assembly {
            x := callvalue()
        }
    }
}
// ----
// TypeError: (81-92): "msg.value" and "callvalue()" can only be used in payable public functions. Make the function "payable" or use an internal function to avoid this error.
