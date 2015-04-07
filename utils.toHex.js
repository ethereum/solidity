var chai = require('chai');
var utils = require('../lib/utils/utils');
var BigNumber = require('bignumber.js');
var assert = chai.assert;

var tests = [
    { value: 1, expected: '0x1' },
    { value: '1', expected: '0x1' },
    { value: '0x1', expected: '0x1'},
    { value: '15', expected: '0xf'},
    { value: '0xf', expected: '0xf'},
    { value: -1, expected: '-0x1'},
    { value: '-1', expected: '-0x1'},
    { value: '-0x1', expected: '-0x1'},
    { value: '-15', expected: '-0xf'},
    { value: '-0xf', expected: '-0xf'},
    { value: '0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd', expected: '0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd'},
    { value: '-0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff', expected: '-0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'},
    { value: '-0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd', expected: '-0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd'},
    { value: 0, expected: '0x0'},
    { value: '0', expected: '0x0'},
    { value: '0x0', expected: '0x0'},
    { value: -0, expected: '0x0'},
    { value: '-0', expected: '0x0'},
    { value: '-0x0', expected: '0x0'},
    { value: [1,2,3,{test: 'data'}], expected: '0x5b312c322c332c7b2274657374223a2264617461227d5d'},
    { value: {test: 'test'}, expected: '0x7b2274657374223a2274657374227d'},
    { value: '{"test": "test"}', expected: '0x7b2274657374223a202274657374227d'},
    { value: 'myString', expected: '0x6d79537472696e67'},
    { value: new BigNumber(15), expected: '0xf'},
    { value: true, expected: '0x1'},
    { value: false, expected: '0x0'}
];

describe('lib/utils/utils', function () {
    describe('toHex', function () {
        tests.forEach(function (test) {
            it('should turn ' + test.value + ' to ' + test.expected, function () {
                assert.strictEqual(utils.toHex(test.value), test.expected);
            });
        });
    });
});

