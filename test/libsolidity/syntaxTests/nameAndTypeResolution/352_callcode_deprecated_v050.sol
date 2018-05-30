pragma experimental "v0.5.0";
contract test {
    function f() pure public {
        address(0x12).callcode;
    }
}
// ----
// TypeError: (85-107): "callcode" has been deprecated in favour of "delegatecall".
