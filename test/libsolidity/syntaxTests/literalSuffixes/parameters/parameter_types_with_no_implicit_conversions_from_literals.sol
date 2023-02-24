type T is uint;
interface I {}
enum E { A, B, C }
struct S { uint x; }
contract C {}

function payableSuffix(address payable) pure suffix returns (uint) {}
function internalFunctionSuffix(function () internal) pure suffix returns (uint) {}
function externalFunctionSuffix(function () external) pure suffix returns (uint) {}
function mappingSuffix(mapping(uint => uint) storage) pure suffix returns (uint) {}
function udvtSuffix(T) pure suffix returns (uint) {}
function enumSuffix(E) pure suffix returns (uint) {}
function structSuffix(S memory) pure suffix returns (uint) {}
function interfaceSuffix(I) pure suffix returns (uint) {}
function contractSuffix(C) pure suffix returns (uint) {}
function staticArraySuffix(uint[3] memory) pure suffix returns (uint) {}
function dynamicArraySuffix(uint[] memory) pure suffix returns (uint) {}
// ----
// TypeError 2998: (109-124): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
// TypeError 2998: (188-209): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
// TypeError 2998: (272-293): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
// TypeError 2998: (347-376): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
// TypeError 2998: (428-429): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
// TypeError 2998: (481-482): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
// TypeError 2998: (536-544): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
// TypeError 2998: (601-602): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
// TypeError 2998: (658-659): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
// TypeError 2998: (718-732): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
// TypeError 2998: (792-805): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
