contract test {
    function f() pure public {
        address(0x12).callcode;
    }
}
// ----
// Warning: (55-77): "callcode" has been deprecated in favour of "delegatecall".
