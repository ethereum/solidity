var chai = require('chai');
var assert = chai.assert;
var formatters = require('../lib/web3/formatters');

var tests = [
    { value: 'latest', expected: 'latest' },
    { value: 'pending', expected: 'pending' },
    { value: 'earliest', expected: 'earliest' },
    { value: 1, expected: '0x1' },
    { value: '0x1', expected: '0x1' }
];

describe('lib/web3/formatters', function () {
    describe('inputDefaultBlockNumberFormatter', function () {
        tests.forEach(function (test) {
            it('should turn ' + test.value + ' to ' + test.expected, function () {
                assert.strictEqual(formatters.inputDefaultBlockNumberFormatter(test.value), test.expected);
            });
        });
    });
});



