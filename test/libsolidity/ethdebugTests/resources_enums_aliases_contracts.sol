type Price is uint128;
enum Color { Red, Green, Blue }
interface I { function f() external; }
library L { function id(uint256 a) internal pure returns (uint256) { return a; } }

contract C {
    Color color;
    Price price;
    I other;
    C self;
    function (uint256) internal pure returns (uint256) internalFunction;
    function (uint256) external returns (bool) externalFunction;
    function g(uint256 a) public pure returns (uint256) { return L.id(a); }
}
// ----
// .resources.types | keys: <IGNORE>
// .resources.types.t_uint128: {"kind": "uint", "bits": 128}
// .resources.types.t_function_internal_pure$_t_uint256_$returns$_t_uint256_$.internal: true
// .resources.types.t_function_internal_pure$_t_uint256_$returns$_t_uint256_$.contains.parameters.type: {"kind": "tuple", "contains": [{"type": {"id": "t_uint256"}}]}
// .resources.types.t_function_internal_pure$_t_uint256_$returns$_t_uint256_$.contains.returns.type.contains[0].type.id: t_uint256
// .resources.types.t_function_external_nonpayable$_t_uint256_$returns$_t_bool_$.external: true
// .resources.pointers | length: 6
