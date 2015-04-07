var chai = require('chai');
var utils = require('../lib/utils/utils.js');
var assert = chai.assert;

var tests = [
    { obj: function () {}, is: false},
    { obj: new Function(), is: false},
    { obj: 'function', is: false},
    { obj: {}, is: false},
    { obj: '[]', is: true},
    { obj: '[1, 2]', is: true},
    { obj: '{}', is: true},
    { obj: '{"a": 123, "b" :3,}', is: false},
    { obj: '{"c" : 2}', is: true}
];

describe('lib/utils/utils', function () {
    describe('isJson', function () {
        tests.forEach(function (test) {
            it('shoud test if value ' + test.obj + ' is json: ' + test.is, function () {
                assert.equal(utils.isJson(test.obj), test.is);
            });
        });   
    });
});

