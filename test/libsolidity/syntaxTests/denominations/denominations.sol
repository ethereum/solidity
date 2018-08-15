contract C {
	uint constant a = 1 sun + 2 trx;
	uint constant b = 1 seconds + 2 minutes + 3 hours + 4 days + 5 weeks + 6 years;
	uint constant c = 2 sun / 1 seconds + 3 trx * 3 hours;
}
// ----
// Warning: (142-149): Using "years" as a unit denomination is deprecated.
