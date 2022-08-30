contract C {
  function g() external {}
  function comparison_operators_for_external_function_pointers_with_dirty_bits() external returns (bool) {
        function() external g_ptr_dirty = this.g;
        assembly {
                g_ptr_dirty.address := or(g_ptr_dirty.address, shl(160, sub(0,1)))
                g_ptr_dirty.selector := or(g_ptr_dirty.selector, shl(32, sub(0,1)))
        }
        function() external g_ptr = this.g;
        return g_ptr == g_ptr_dirty;
  }
}
// ====
// EVMVersion: >=constantinople
// ----
// comparison_operators_for_external_function_pointers_with_dirty_bits() -> true
