contract C {
    function f() view public {
        msg.value;
    }
}
// ----
// TypeError: (52-61): "msg.value" and "callvalue()" can only be used in payable public functions. Make the function "payable" or use an internal function to avoid this error.
