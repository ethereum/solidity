var chai = require('chai');
var utils = require('../lib/utils/utils.js');
var assert = chai.assert;

var tests = [
    { value: function () {}, is: false},
    { value: new Function(), is: false},
    { value: 'function', is: true},
    { value: {}, is: false},
    { value: new String('hello'), is: true}
];

describe('lib/utils/utils', function () {
    describe('isString', function () {
        tests.forEach(function (test) {
            it('shoud test if value ' + test.func + ' is string: ' + test.is, function () {
                assert.equal(utils.isString(test.value), test.is);
            });
        });   
    });
});

