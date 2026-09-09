contract C {
    function internalFunction(uint a, uint b, uint c) internal returns (uint r) { r = a * 100 + b * 10 + c * 1; }
    function publicFunction(uint a, uint b, uint c) public returns (uint r) { r = a * 100 + b * 10 + c * 1; }
    function externalFunction(uint a, uint b, uint c) public returns (uint r) { r = a * 100 + b * 10 + c * 1; }

    function internalOrdered() public returns (uint r) { r = internalFunction({a: 1, b: 2, c: 3}); }
    function internalUnordered() public returns (uint r) { r = internalFunction({b: 2, c: 3, a: 1}); }
    function publicOrdered() public returns (uint r) { r = publicFunction({a: 1, b: 2, c: 3}); }
    function publicUnordered() public returns (uint r) { r = publicFunction({b: 2, c: 3, a: 1}); }
    function externalOrdered() public returns (uint r) { r = externalFunction({a: 1, b: 2, c: 3}); }
    function externalUnordered() public returns (uint r) { r = externalFunction({b: 2, c: 3, a: 1}); }
}
// ----
// internalOrdered() -> 123
// internalUnordered() -> 123
// publicOrdered() -> 123
// publicUnordered() -> 123
// externalOrdered() -> 123
// externalUnordered() -> 123
