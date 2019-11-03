# OS_Homework2

Homework2 for Operating Systems Class on Interprocess Communication.

For info about implementation consult Readme.pdf in this repository.

## Assignment requirements in brief.

1 x Process P.

N x process C's.

Process P reads random lines from a text file, send the lines to process C's and waits for them to return a MD5 hash.
Each idle process C undertakes a line and calculates its MD5 hash and returns the hash to the process P.
