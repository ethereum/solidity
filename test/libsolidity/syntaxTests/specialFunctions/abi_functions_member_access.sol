contract C {
    function f() public pure {
        abi.encode;
        abi.encodePacked;
        abi.encodeWithSelector;
        abi.encodeWithSignature;
        abi.decode;
    }
}
// ----
// Warning 6133: (52-62='abi.encode'): Statement has no effect.
// Warning 6133: (72-88='abi.encodePacked'): Statement has no effect.
// Warning 6133: (98-120='abi.encodeWithSelector'): Statement has no effect.
// Warning 6133: (130-153='abi.encodeWithSignature'): Statement has no effect.
// Warning 6133: (163-173='abi.decode'): Statement has no effect.
