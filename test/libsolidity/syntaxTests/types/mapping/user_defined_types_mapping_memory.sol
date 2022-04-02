type T is uint;
library L {
  function f(mapping(T=>T) memory) public {}
}
// ----
// TypeError 4061: (41-61='mapping(T=>T) memory'): Type mapping(T => T) is only valid in storage because it contains a (nested) mapping.
