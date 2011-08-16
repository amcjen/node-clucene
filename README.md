OVERVIEW
=========
node-clucene is a native Node.js module that wraps up the CLucene library.	CLucene is a C++ port of the ubiquitous Java Lucene library.	The Lucene ecosystem is vibrant, and lots of great code has come out of it.	 However, for those who do not wish to run a JVM-based information retrieval system, CLucene makes a lot of sense.

CLucene is based on a port of Lucene 2.3.	 Unfortunately, there's been a lot of new updates and features added since then.	The goal is to add features to this module as necessary, without rewriting it to map method-to-method with the current version of Lucene.

Right now, adding a document with the same docId will *replace* the older document in the index that has the same docId.  This is intentional, and makes document updating simple.  In the future, this will probably get refactored into a separate updateDocument() method call instead, and revert addDocument() back to adding only.

This library begun as a project from Tyler Gillies (http://github.com/tjgillies/node-lucene).	 Mad props to him for getting this library bootstrapped and running!


HOW IT WORKS:
============

Indexing information into the index
-------------------------------
		var cl = require('clucene').CLucene;
		var doc = new cl.Document();
		
		doc.addField('name', 'Eric Jennings', cl.Store.STORE_YES|cl.Index.INDEX_TOKENIZED);
		doc.addField('timestamp', 'Eric Jennings', cl.Store.STORE_YES|cl.Index.INDEX_UNTOKENIZED);
		
		lucene.addDocument(docId, doc, '/path/where/to/store/index', function(err, indexTime, docsReplaced) {
				if (err) {
						console.log('Error indexing document: ' + err);
				}
				
				console.log('Indexed document in ' + indexTime + ' ms');
				
				if (docsReplaced > 0) {
						console.log('Updated ' + docsReplaced + ' existing document(s)');
				}
		});


Querying information out of the index
-------------------------------
		var cl = require('./clucene').CLucene;
		var lucene = new cl.Lucene();
		var util = require('util');
		
		var queryTerm = 'name:Eri*'

		lucene.search('/path/where/index/stored', queryTerm, function(err, results) {
				if (err) {
						console.log('Search error: ' + err);
						return;
				}
				
				console.log('Search results: ');
				
				for (var i=0; i<results.length; i++) {
					console.log(results[i]);
				}
		});
		

REQUIREMENTS:
=============
node-clucene requires the CLucene library.	This is not included in this module, you must install it on your own.	 Instructions can be found here (http://clucene.sourceforge.net/)
