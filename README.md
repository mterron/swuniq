# ![swuniq](https://i.imgur.com/LpN432Z.png)swuniq
Deduplicate matching lines (within a configurable window) from a file or standard input, writing to standard output.

Like uniq but works on unsorted input to be used as a pipe filter with constant memory usage.

#### Memory Usage
Uses a ringbuffer of configurable size (-w option) as a FIFO queue to store hashes of each line to keep memory use constant (64bits * -w value).


#### Example
```sh
# swuniq -h
Usage: swuniq [-w N] [INPUT]
Filter matching lines (within a configurable window) from INPUT 
(or standard input), writing to standard output.

	-w N Size of the sliding window to use for deduplication
 Note: By default swuniq will use a window of 100 lines.

# cat input.txt 
1
1
1
2
2
3
4
1
2
3
4
5
6
7
8
6
8
5
2
2
2
1
5

# swuniq -w 4 < input.txt
1
2
3
4
5
6
7
8
2
1
5

# swuniq -w 2 < input.txt 
1
2
3
4
1
2
3
4
5
6
7
8
6
5
2
1
5
 
```
