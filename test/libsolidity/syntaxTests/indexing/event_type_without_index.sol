contract MyContract {
    event MyCustomEvent(uint256);
    function test() public {
        MyCustomEvent[];
    }
}
// ----
// TypeError 2614: (93-106): Indexed expression has to be a type, mapping or array (is event MyCustomEvent(uint256))
