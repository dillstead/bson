# BSON
BSON is a simplified variant of JSON

This project parses BSON into objects, lists, integers, and strings:
```
bson
    element

element
    ws value ws

value
    object
    array
    string
    integer

object
    '{' ws '}'
    '{' members '}'

members
    member
    member ',' members

member
    ws string ws ':' element

array
    '[' ws ']'
    '[' elements ']'

elements
    element
    element ',' elements

element
    ws value ws

string
    '"' characters '"'

characters
    0x20 ... 0x7E
    '\' escape

escape
    '\'
    'n'
    '"'
    
integer
    digits
    '-' digits

digits
    digit
    digit digits
    
digits
    '0' ... '9'    

ws
    ""
    0x09
    0x0A
    0x0B
    0x0C
    0x0D
    0x20
```


To build the project:
```
$ make NDEBUG=1 NASAN=1 all
```
The default build builds a static parsing library, a dynamic parsing library, a test program, and an example program.

To run the tests:
```
$ ./bson_test 
All tests passed
```

To run the example:
```
$ echo '{ "a" : "b" }' > test.bson
$ ./bson_example test.bson 
{"a":"b"}
$ echo '{ "a" : }' > test.bson
$ ./bson_example test.bson 
Syntax error row 1 col 9
```

The public API can be found in ```bson.h``` and is simple to use.  '''example.c``` exercises all of the API functions and can be used as a reference.

