var testCase = require('nodeunit').testCase,
    clucene = require('../clucene').CLucene,
    lucene = clucene.Lucene();
    
var doc = null;
var index = './temp.index';

module.exports = testCase({
    test_new_document: function(assert) {
        assert.expect(1);
        doc = new cl.Document();
        assert.notEquals(null, doc, 'doc should not be null');
        assert.done();
    },
    test_add_field_to_document: function(assert) {
        assert.notEquals(null, doc, 'document should not be null');
        assert.doesNotThrow(function() {
          doc.addField('key1', 'value1', config);
        }, 'Should not throw an Error.');
        assert.done();
    },
    test_add_document_to_index: function(assert) {
        assert.doesNotThrow(function() {
          lucene.addDocument(doc, index, function(err, indexTime) {
            assert.equals(err, null, 'addDocument should not return an error');
          });
        }, 'Should not throw an Error');
        assert.done();
    }
});