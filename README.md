# ![swuniq](https://i.imgur.com/LpN432Z.png)swuniq
Like uniq but works on unsorted input to be used as a pipe filter.
Allows you to define the how many lines back it'll look for duplicates.

### Performance
Performance is directly related to the size of the buffer as looking up a hash has O(n).

#### Memory Usage
Uses a ringbuffer of configurable size (-w option) as a FIFO queue to store hashes of each line to keep memory use constant (64bits * # lines).



```sh
# swuniq -h
Usage: swuniq [-w N]

Filters matching lines from stdin (within a configurable window)
writing to stdout.

By default swuniq will filter out lines that match any of the
previous 100 lines.

	-w N Size of the sliding window to use for deduplication
  
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
