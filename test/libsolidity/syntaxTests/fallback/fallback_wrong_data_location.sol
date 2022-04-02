contract D {
    fallback(bytes memory) external returns (bytes memory) {}
}
contract E {
    fallback(bytes memory) external returns (bytes calldata) {}
}
contract F {
    fallback(bytes calldata) external returns (bytes calldata) {}
}
// ----
// TypeError 5570: (57-71='(bytes memory)'): Fallback function either has to have the signature "fallback()" or "fallback(bytes calldata) returns (bytes memory)".
// TypeError 5570: (134-150='(bytes calldata)'): Fallback function either has to have the signature "fallback()" or "fallback(bytes calldata) returns (bytes memory)".
// TypeError 5570: (215-231='(bytes calldata)'): Fallback function either has to have the signature "fallback()" or "fallback(bytes calldata) returns (bytes memory)".
