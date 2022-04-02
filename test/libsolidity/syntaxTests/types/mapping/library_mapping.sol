library L {}
contract C
{
  mapping(bool => L) j;
  mapping(L => bool) i;
}
// ----
// TypeError 1130: (44-45='L'): Invalid use of a library name.
// TypeError 1130: (60-61='L'): Invalid use of a library name.
