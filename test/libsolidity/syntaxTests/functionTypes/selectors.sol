contract B {
    function ext() external {}
    function pub() public {}
}

contract C is B {
    function test() public pure {
        B.ext.selector;
        B.pub.selector;
        this.ext.selector;
        pub.selector;
    }
}

contract D {
    function test() public pure {
        B.ext.selector;
        B.pub.selector;
    }
}
// ----
// Warning 6133: (136-150='B.ext.selector'): Statement has no effect.
// Warning 6133: (160-174='B.pub.selector'): Statement has no effect.
// Warning 6133: (184-201='this.ext.selector'): Statement has no effect.
// Warning 6133: (289-303='B.ext.selector'): Statement has no effect.
// Warning 6133: (313-327='B.pub.selector'): Statement has no effect.
