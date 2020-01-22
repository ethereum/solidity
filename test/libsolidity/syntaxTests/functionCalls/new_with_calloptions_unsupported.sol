contract D { constructor() public payable {} }
contract C {
    function foo() pure internal {
		new D{salt:"abc", value:3};
		new D{salt:"abc"};
		new D{value:5+5};
		new D{salt:"aabbcc"};
    }
}
// ====
// EVMVersion: <constantinople
// ----
// TypeError: (97-123): Unsupported call option "salt" (requires Constantinople-compatible VMs).
// TypeError: (127-144): Unsupported call option "salt" (requires Constantinople-compatible VMs).
// TypeError: (168-188): Unsupported call option "salt" (requires Constantinople-compatible VMs).
