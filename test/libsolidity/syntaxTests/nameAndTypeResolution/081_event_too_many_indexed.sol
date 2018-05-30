contract c {
    event e(uint indexed a, bytes3 indexed b, bool indexed c, uint indexed d);
}
// ----
// TypeError: (17-91): More than 3 indexed arguments for event.
